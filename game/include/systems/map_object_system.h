#pragma once

#include "system.h"
#include "raylib.h"

class MapObjectComponent;
class TriggerComponent;
class SceneRenderSystem;

class MapObjectSystem : public System
{
public:
    DEFINE_SYSTEM(MapObjectSystem)

    std::vector<MapObjectComponent*> MapObjects;
    std::vector<TriggerComponent*> Triggers;

    bool MoveEntity(Vector3& position, Vector3& desiredMotion, float radius, GameObject* entity = nullptr);
    void CheckTriggers(GameObject* entity, float radius, bool hitSomething);

protected:
    void OnSetup() override;
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;
    void OnRemoveObject(GameObject* object) override;

    SceneRenderSystem* SceneRenderer = nullptr;
};