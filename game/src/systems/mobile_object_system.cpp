#include "systems/mobile_object_system.h"

#include "components/mob_behavior_component.h"
#include "components/transform_component.h"
#include "utilities/collision_utils.h"

void MobSystem::OnUpdate()
{
    // do AI updates

    for (auto& behavior : MobBehaviors.Components)
    {
        behavior->Process();
    }
}

void MobSystem::OnAddObject(GameObject* object)
{
    Mobs.Add(object);
    MobBehaviors.Add(object);
}

void MobSystem::OnRemoveObject(GameObject* object)
{
    MobBehaviors.Remove(object);
    Mobs.Remove(object);
}
