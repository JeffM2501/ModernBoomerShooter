#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"

class TransformComponent : public Component
{
private:
public:
    DEFINE_COMPONENT(TransformComponent)

    Vector3 Position = Vector3Zeros;

    Vector3 Forward = { 0 };

    inline void Rotate(float angle)
    {
        Vector2 rot = Vector2Rotate(Vector2{ Forward.x, Forward.y }, angle * DEG2RAD);
        Forward.x = rot.x;
        Forward.y = rot.y;
    }

    inline void SetFacing(float angle) { Forward = Vector3{ cosf((angle +90) * DEG2RAD), sinf((angle +90) * DEG2RAD), 0 }; }
    inline float GetFacing() const { return atan2f(Forward.y, Forward.x) * RAD2DEG - 90; }

};