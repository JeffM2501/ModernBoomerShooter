#include "systems/player_management_system.h"

#include "services/game_time.h"
#include "systems/input_system.h"
#include "components/spawn_point_component.h"
#include "components/transform_component.h"

#include "world.h"

void PlayerManagementSystem::OnSetup()
{
    if (!WorldPtr)
        return;

    Input = WorldPtr->GetSystem<InputSystem>();

    PlayerPos = Vector3Zeros;
    PlayerFacing = Vector3UnitZ;
    PlayerYaw = 0;
    PlayerPitch = 0;

    if (Spawn)
    {
        TransformComponent& transform = Spawn->GetOwner()->MustGetComponent<TransformComponent>();
        PlayerPitch = transform.Facing;
        PlayerPos = transform.Position;
    }
}

void PlayerManagementSystem::OnAddObject(GameObject* object)
{
    if (object->HasComponent<SpawnPointComponent>())
        Spawn = object->GetComponent<SpawnPointComponent>();
}

void PlayerManagementSystem::OnUpdate()
{
    if (!WorldPtr || !Input)
        return;
    
#if defined(_DEBUG)
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
#endif
    {
        PlayerYaw += Input->GetActionValue(Actions::Yaw);
        PlayerPitch += Input->GetActionValue(Actions::Pitch);

        // unwind angle
        while (PlayerYaw > 180)
            PlayerYaw -= 360;
        while (PlayerYaw < -180)
            PlayerYaw += 360;

        // clamp
        if (PlayerPitch > 89)
            PlayerPitch = 89;
        if (PlayerPitch < -89)
            PlayerPitch = -89;
    }
    
    PlayerFacing = Vector3RotateByAxisAngle(Vector3UnitY, Vector3UnitZ, PlayerYaw * DEG2RAD);

    float speed = (PlayerFowardSpeed * GameTime::Scale(Input->GetActionValue(Actions::Forward)));
    if (speed < 0)
        speed *= 0.5f;

    Vector3 forward = PlayerFacing * speed;
    Vector3 sideways = Vector3RotateByAxisAngle(PlayerFacing, Vector3UnitZ, -90 * DEG2RAD);

    sideways *= (PlayerSideStepSpeed * GameTime::Scale(Input->GetActionValue(Actions::Sideways)));

    Vector3 motion = forward + sideways;

    bool isGhost = true;
    if (!isGhost)
    {
        WorldPtr->GetMap().MoveEntity(PlayerPos, motion, 0.25f);
    }
    else
    {
        PlayerPos += motion;
    }
}

