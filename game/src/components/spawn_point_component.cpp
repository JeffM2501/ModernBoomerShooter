#include "components/spawn_point_component.h"

void SpawnPointComponent::OnAddedToObject()
{
    AddToSystem<PlayerManagementSystem>();
}
