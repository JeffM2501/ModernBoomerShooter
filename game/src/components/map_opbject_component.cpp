#include "components/map_object_component.h"
#include "systems/map_object_system.h"

void MapObjectComponent::OnAddedToObject()
{
    AddToSystem<MapObjectSystem>();
}
