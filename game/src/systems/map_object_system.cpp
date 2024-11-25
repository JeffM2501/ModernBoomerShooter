#include "systems/map_object_system.h"

#include "world.h"
#include "systems/scene_render_system.h"
#include "components/map_object_component.h"
#include "services/model_manager.h"


void MapObjectSystem::OnSetup()
{
    SceneRenderer = WorldPtr->GetSystem<SceneRenderSystem>();
}

void MapObjectSystem::OnUpdate()
{
    // AI updates?
}

void MapObjectSystem::OnAddObject(GameObject* object)
{
    if (object->HasComponent<MapObjectComponent>())
    {
        MapObjects.push_back(object->GetComponent<MapObjectComponent>());

        MapObjectComponent* mapObject = MapObjects.back();

        mapObject->Instance = ModelManager::GetModel(mapObject->ModelName);

        if (SceneRenderer)
            SceneRenderer->MapObjectAdded(mapObject);
    }
}
