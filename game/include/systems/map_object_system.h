#pragma once

#include "system.h"
#include "raylib.h"

class MapObjectComponent;
class SceneRenderSystem;

class MapObjectSystem : public System
{
public:
    DEFINE_SYSTEM(MapObjectSystem)

    std::vector<MapObjectComponent*> MapObjects;

    bool MoveEntity(Vector3& position, Vector3& desiredMotion, float radius);

protected:
    void OnSetup() override;
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;

    SceneRenderSystem* SceneRenderer = nullptr;
};