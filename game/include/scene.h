#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include "game_object.h"
#include "system.h"
#include "map/map.h"
#include "map/raycaster.h"

#include "game.h"

class Scene
{
public:
    Scene();

    void Init();
    void Cleanup();

    void Load(std::string_view map);
    void ReloadMap();

    GameObject* AddObject();

    Map& GetMap() { return WorldMap; }
    const Map& GetMap() const { return WorldMap; }

    Raycaster& GetRaycaster() { return WorldRaycaster; }

protected:
    std::unique_ptr<GameObject> RootObject;

    Map WorldMap;
    Raycaster WorldRaycaster;

    std::string CurrentWorldMap;
};