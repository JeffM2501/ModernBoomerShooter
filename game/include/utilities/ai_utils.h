#pragma once

#include "raylib.h"
#include "raymath.h"

namespace AIUtils
{
    float GetRotationTo(const Vector3& position, const Vector3& facing, const Vector3& target);

    Vector3 MoveTo(const Vector3& position, Vector3& facing, const Vector3& target, float maxSpeed, float maxRotation);
}