#pragma once

#include "system.h"
#include "raylib.h"
#include "raymath.h"

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

protected:
    class InputSystem* Input = nullptr;

    float PlayerFowardSpeed = 15;
    float PlayerSideStepSpeed = 8;
};