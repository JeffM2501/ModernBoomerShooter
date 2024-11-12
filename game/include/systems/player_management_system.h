#pragma once

#include "system.h"
#include "raylib.h"
#include "raymath.h"

class SpawnPointComponent;

class PlayerManagementSystem : public System
{
public:
    DEFINE_SYSTEM(PlayerManagementSystem)

    Vector3 PlayerPos = Vector3Zeros;
    Vector3 PlayerFacing = Vector3UnitZ;

    float PlayerYaw = 0;
    float PlayerPitch = 0;

protected:
    void OnSetup() override;
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;

protected:
    class InputSystem* Input = nullptr;

    float PlayerFowardSpeed = 8;
    float PlayerSideStepSpeed = 4;

    SpawnPointComponent* Spawn = nullptr;
};