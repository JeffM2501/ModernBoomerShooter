#pragma once

#include "system.h"
#include "map/map_render.h"

class SceneRenderSystem : public System
{
public:
    DEFINE_SYSTM_NO_CONSTRUCTOR(SceneRenderSystem);

    SceneRenderSystem(World* world);

protected:
    void OnSetup() override;
    void OnUpdate() override;

protected:

    MapRenderer Render;
    class PlayerManagementSystem* PlayerManager = nullptr;
};