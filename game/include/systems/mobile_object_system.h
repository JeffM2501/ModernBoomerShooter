#pragma once
#include "system.h"
#include "game_object.h"

#include "components/mobile_object_component.h"

class MobBehaviorComponent;

class MobSystem : public System
{
public:
    DEFINE_SYSTEM(MobSystem);

    SystemComponentList<MobComponent> Mobs;
    SystemComponentList<MobBehaviorComponent> MobBehaviors;

protected:
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;
    void OnRemoveObject(GameObject* object) override;

protected:
};