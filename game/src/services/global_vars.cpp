#include "services/global_vars.h"

namespace GlobalVars
{
#if defined(_DEBUG)
    static constexpr bool DebugTrue = true;
#else
    static constexpr bool DebugTrue = false;
#endif

#if defined(_DEBUG)
    static constexpr bool DebugFalse = false;
#else
    static constexpr bool DebugFalse = true;
#endif

    bool UseGhostMovement = false;
    bool UseVisCulling = true;
    bool ShowCollisionVolumes = false;
    bool ShowCoordinates = DebugTrue;

    bool UseVSync = DebugFalse;
    int FPSCap = 300;

    bool UseMouseDrag = DebugTrue;
}