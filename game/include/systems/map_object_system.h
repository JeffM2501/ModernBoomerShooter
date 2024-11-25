#pragma once

#include "system.h"

class MapObjectComponent;
class SceneRenderSystem;

class MapObjectSystem : public System
{
public:
    DEFINE_SYSTEM(MapObjectSystem)

    std::vector<MapObjectComponent*> MapObjects;

protected:
    void OnSetup() override;
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;

    SceneRenderSystem* SceneRenderer = nullptr;
};