#pragma once

#include "component.h"
#include "game_object.h"

#include "raylib.h"
#include "map/map.h"


#include "systems/map_object_system.h"

class DoorControllerComponent : public Component
{
public:
    DEFINE_COMPONENT(DoorControllerComponent)

    DoorControllerComponent(GameObject* owner, size_t doorId) : Component(owner) { Doors.push_back(doorId); }

    std::vector<size_t> Doors = { 0 };

    void OnAddedToObject() override;

    void Update();

    static constexpr char DoorOpening[] = "DoorOpening";
    static constexpr char DoorOpened[] = "DoorOpened";
    static constexpr char DoorClosing[] = "DoorClosing";
    static constexpr char DoorClosed[] = "DoorClosed";

    static constexpr char DoorSecuritySuccess[] = "DoorSecuritySuccess";
    static constexpr char DoorSecurityFailed[] = "DoorSecurityFailed";

protected:
    void OnTriggerEnter(GameObject* sender, GameObject* subject);
    void OnTriggerExit(GameObject* sender, GameObject* subject);

    void SetDoorParams(float param);
    void SetDoorBlocked(bool blocked);

    enum class State
    {
        Closed,
        Opening,
        Open,
        Closing
    };

    State OpenState = State::Closed;

    bool NeedCloseASAP = false;

    float Param = 0;

    float OpenSpeed = 1;
    float CloseSpeed = 1;

    bool MustOpenBeforClose = true;
};