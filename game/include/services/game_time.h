#pragma once
#include "raylib.h"

namespace GameTime
{
    extern float NominalFPS;

    inline float GetDeltaTime()
    {
#if defined(_DEBUG)
        return 1.0f / NominalFPS;
#else
        return ::GetFrameTime();
#endif
    }

    inline float Scale(float value)
    {
        return value * GetDeltaTime();
    }

    void ComputeNominalFPS();
}