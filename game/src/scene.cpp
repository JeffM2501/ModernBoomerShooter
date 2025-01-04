#include "scene.h"

#include "system.h"
#include "game_object.h"

#include "map/map_reader.h"

Scene::Scene()
{
    RootObject = std::make_unique<GameObject>();
}

void Scene::Init()
{
    WorldRaycaster.SetMap(&WorldMap);
}

void Scene::Load(std::string_view map)
{
    CurrentWorldMap = map;
    WorldMap.Clear();

    if (!map.empty())
        ReadWorld(map.data(), *this);

    WorldRaycaster.SetMap(&WorldMap);
}

void Scene::ReloadMap()
{
    WorldMap.Clear();
    if (!CurrentWorldMap.empty())
        ReadWorld(CurrentWorldMap.data(), *this);

    WorldRaycaster.SetMap(&WorldMap);
}

void Scene::Cleanup()
{
    RootObject = nullptr;
}

GameObject* Scene::AddObject()
{
    auto* object = RootObject->AddChild();

    return object;
}