#include "components/mobile_object_component.h"
#include "systems/mobile_object_system.h"

void MobComponent::OnAddedToObject()
{
    AddToSystem<MobSystem>();
}
