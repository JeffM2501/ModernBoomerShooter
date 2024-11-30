#include "systems/player_management_system.h"

#include "services/game_time.h"
#include "systems/input_system.h"
#include "systems/map_object_system.h"
#include "components/player_info_component.h"
#include "components/spawn_point_component.h"
#include "components/transform_component.h"
#include "services/global_vars.h"

#include "world.h"

void PlayerManagementSystem::OnSetup()
{
    if (!WorldPtr)
        return;

    Input = WorldPtr->GetSystem<InputSystem>();
    MapObjects = WorldPtr->GetSystem<MapObjectSystem>();

    if (PlayerObject == nullptr)
    {
        PlayerObject = WorldPtr->AddObject();
        PlayerTransform = PlayerObject->AddComponent<TransformComponent>();
    }

    PlayerObject->MustGetComponent<PlayerInfoComponent>().PlayerId = 0;

    PlayerPitch = 0;

    if (Spawn)
    {
        TransformComponent& transform = Spawn->GetOwner()->MustGetComponent<TransformComponent>();
        PlayerTransform->Position = transform.Position;
        PlayerTransform->Facing = transform.Facing;
    }
}

Vector3 PlayerManagementSystem::GetPlayerPos() const
{
    if (PlayerTransform)
        return PlayerTransform->Position;

    return Vector3Zeros;
}

Vector3 PlayerManagementSystem::GetPlayerFacing() const
{
    if (PlayerTransform)
        return PlayerTransform->GetForward();

    return Vector3UnitY;
}

float PlayerManagementSystem::GetPlayerYaw() const
{
    if (PlayerTransform)
        return PlayerTransform->Facing;

    return 0;
}

float PlayerManagementSystem::GetPlayerPitch() const
{
    return PlayerPitch;
}

void PlayerManagementSystem::OnAddObject(GameObject* object)
{
    if (object->HasComponent<SpawnPointComponent>())
        Spawn = object->GetComponent<SpawnPointComponent>();
}

void PlayerManagementSystem::OnUpdate()
{
    if (!WorldPtr || !Input || !PlayerObject)
        return;
    
    if (!GlobalVars::UseMouseDrag || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        PlayerTransform->Facing += Input->GetActionValue(Actions::Yaw);
        PlayerPitch += Input->GetActionValue(Actions::Pitch);

        // unwind angle
        while (PlayerTransform->Facing > 180)
            PlayerTransform->Facing -= 360;
        while (PlayerTransform->Facing < -180)
            PlayerTransform->Facing += 360;

        // clamp
        if (PlayerPitch > 89)
            PlayerPitch = 89;
        if (PlayerPitch < -89)
            PlayerPitch = -89;
    }
    
    float speed = (PlayerFowardSpeed * GameTime::Scale(Input->GetActionValue(Actions::Forward)));
    if (speed < 0)
        speed *= 0.5f;

    Vector3 forward = PlayerTransform->GetForward() * speed;
    Vector3 sideways = Vector3RotateByAxisAngle(PlayerTransform->GetForward(), Vector3UnitZ, -90 * DEG2RAD);

    sideways *= (PlayerSideStepSpeed * GameTime::Scale(Input->GetActionValue(Actions::Sideways)));

    Vector3 motion = forward + sideways;

    static constexpr float playerRadius = 0.25f;

    bool hitWall = false;
    bool hitObstacle = false;
    if (!GlobalVars::UseGhostMovement)
    {
        hitWall = WorldPtr->GetMap().MoveEntity(PlayerTransform->Position, motion, playerRadius);
        hitObstacle = MapObjects->MoveEntity(PlayerTransform->Position, motion, playerRadius);

        if (hitWall || hitObstacle)
        {
            // trigger event ?
        }
    }

    PlayerTransform->Position += motion;

    MapObjects->CheckTriggers(PlayerObject, playerRadius, hitWall || hitObstacle);
}