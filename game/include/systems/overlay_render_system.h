#pragma once
#include "system.h"

class OverlayRenderSystem : public System
{
public:
    DEFINE_SYSTEM(OverlayRenderSystem);

protected:
    void OnUpdate() override;
};