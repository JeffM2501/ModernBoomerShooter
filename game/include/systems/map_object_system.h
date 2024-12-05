#pragma once

#include "system.h"
#include "systems/audio_system.h"
#include "components/map_object_component.h"
#include "component.h"
#include "raylib.h"

#include <vector>

class TriggerComponent;
class DoorControllerComponent;
class SceneRenderSystem;


class MapObjectSystem : public System
{
public:
    DEFINE_SYSTEM(MapObjectSystem)

    SystemComponentList<MapObjectComponent> MapObjects;
    SystemComponentList<TriggerComponent> Triggers;
    SystemComponentList<DoorControllerComponent> Doors;

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