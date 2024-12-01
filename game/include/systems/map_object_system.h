#pragma once

#include "system.h"
#include "systems/audio_system.h"
#include "raylib.h"

#include <vector>

class MapObjectComponent;
class TriggerComponent;
class DoorControllerComponent;
class SceneRenderSystem;


class MapObjectSystem : public System
{
public:
    DEFINE_SYSTEM(MapObjectSystem)

    std::vector<MapObjectComponent*> MapObjects;
    std::vector<TriggerComponent*> Triggers;
    std::vector<DoorControllerComponent*> Doors;

    bool MoveEntity(Vector3& position, Vector3& desiredMotion, float radius, GameObject* entity = nullptr);
    void CheckTriggers(GameObject* entity, float radius, bool hitSomething);

protected:
    void OnSetup() override;
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;
    void OnRemoveObject(GameObject* object) override;

    SceneRenderSystem* SceneRenderer = nullptr;
    AudioSystem* Audio = nullptr;

    SoundInstance::Ptr OpenDoorSound;
    SoundInstance::Ptr CloseDoorSound;
};