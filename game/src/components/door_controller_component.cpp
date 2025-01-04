#include "components/door_controller_component.h"
#include "components/trigger_component.h"
#include "systems/map_object_system.h"
#include "services/game_time.h"
#include "scene.h"

void DoorControllerComponent::OnAddedToObject()
{
    AddToSystem<MapObjectSystem>();

    GameObject* owner = GetOwner();

    owner->AddEventHandler(TriggerComponent::TriggerEnter,
        [this](size_t, GameObject* sender, GameObject* subject)
        {
            OnTriggerEnter(sender, subject);
        },
        owner->GetToken());


    owner->AddEventHandler(TriggerComponent::TriggerExit,
        [this](size_t, GameObject* sender, GameObject* subject)
        {
            OnTriggerExit(sender, subject);
        },
        owner->GetToken());
}

void DoorControllerComponent::SetDoorParams(float param)
{
    GameObject* owner = GetOwner();
    Map& map = App::GetScene().GetMap();

    if (param > 1)
        param = 1;
    if (param < 0)
        param = 0;

    for (auto doorId : Doors)
    {
        auto& cell = map.Cells[doorId];
        cell.ParamState = uint8_t(param * 255);
    }
}

void DoorControllerComponent::SetDoorBlocked(bool blocked)
{
    GameObject* owner = GetOwner();
    Map& map = App::GetScene().GetMap();

    for (auto doorId : Doors)
    {
        auto& cell = map.Cells[doorId];

        if (blocked)
            cell.Flags |= MapCellFlags::Impassible;
        else
            cell.Flags &= ~(MapCellFlags::Impassible);
    }
}

void DoorControllerComponent::Update()
{
    if (Doors.empty())
        return;

    switch (OpenState)
    {
    case DoorControllerComponent::State::Open:
        // force close
        if (NeedCloseASAP)
        {
            if (MiniumOpenTime > 0)
            {
                Param = MiniumOpenTime;
                OpenState = State::WaitingForClose;
            }
            else
            {
                NeedCloseASAP = false;
                OpenState = State::Closing;
                GetOwner()->CallEvent(DoorClosing, nullptr);
            }
        }
        break;

    case DoorControllerComponent::State::Opening:
        Param += GameTime::GetDeltaTime() / OpenSpeed;
        if (Param >= 1)
        {
            Param = 1;
            OpenState = State::Open;

            SetDoorParams(1);
        }
        else
        {
            SetDoorParams(Param);

            if (Param >= 0.5f)
                SetDoorBlocked(false);
        }
        break;

    case DoorControllerComponent::State::WaitingForClose:
        Param -= GameTime::GetDeltaTime();
        if (Param <= 0)
        {
            Param = 1;
            NeedCloseASAP = false;
            OpenState = State::Closing;
            GetOwner()->CallEvent(DoorClosing, nullptr);
        }
        break;

    case DoorControllerComponent::State::Closing:
        Param -= GameTime::GetDeltaTime() / CloseSpeed;
        if (Param <= 0)
        {
            Param = 0;
            OpenState = State::Closed;

            SetDoorParams(0);
        }
        else
        {
            SetDoorParams(Param);

            if (Param <= 0.5f)
                SetDoorBlocked(true);
        }

        break;

    case DoorControllerComponent::State::Closed:
    default:
        break; // do nothing, we are just chillin closed
    }
}

void DoorControllerComponent::OnTriggerEnter(GameObject* sender, GameObject* subject)
{
    if (Doors.empty())
        return;

    if (OpenState == State::Open || OpenState == State::Opening)
        return; // we are already in the right state, and update will handle it

    // TODO, check security

    // Start the open process
    OpenState = State::Opening;
    NeedCloseASAP = false;
    GetOwner()->CallEvent(DoorOpening, subject);
}

void DoorControllerComponent::OnTriggerExit(GameObject* sender, GameObject* subject)
{
    if (Doors.empty())
        return;

    if (OpenState == State::Closed || OpenState == State::Closing)
        return; // we are already in the right state, and update will handle it

    // do we have a valid trigger, and is anyone else still in it? if so, we can't close on them (TODO, what about security flags? have an 'allow murder' option?)
    TriggerComponent* trigger = sender->GetComponent<TriggerComponent>();
    if (trigger && trigger->HasAnyObjects())
        return;

    if (OpenState == State::Open && StayOpen)
        return;

    // if the door must fully open before closing, then flag it as asap close
    if (MustOpenBeforClose && OpenState == State::Opening)
    {
        NeedCloseASAP = !StayOpen;
        return;
    }

    // start closing the door
    OpenState = State::Closing;
    GetOwner()->CallEvent(DoorClosing, subject);
}
