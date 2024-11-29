#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"

class TransformComponent : public Component
{
public:
    DEFINE_COMPONENT(TransformComponent)

    Vector3 Position = Vector3Zeros;
    float Facing = 0;

    inline Vector3 GetForward() const
    {
        return Vector3RotateByAxisAngle(Vector3UnitY, Vector3UnitZ, Facing * DEG2RAD);
    }
};