#include "services/game_time.h"

namespace GameTime
{
    float NominalFPS = 60.0f;

    void ComputeNominalFPS()
    {
        NominalFPS = float(GetMonitorRefreshRate(GetCurrentMonitor()));
    }
}