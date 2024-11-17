#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include "game_object.h"
#include "system.h"
#include "map/map.h"
#include "map/raycaster.h"

enum class SystemStage
{
    PreUpdate,
    Update,
    PostUpdate,
    Async,
    PreRender,
    Render,
    PostRender,
};

enum class WorldState
{
    Empty,
    Loading,
    Playing,
    Closing,
};

class World
{
public:
    World();
    void RegisterSystem(SystemStage stage, std::unique_ptr<System> system);

    template<class T>
    inline void RegisterSystem(SystemStage stage)
    {
        RegisterSystem(stage, std::move(std::make_unique<T>(this)));
    }

    System* GetSystem(size_t systemId);

    template<class T>
    inline T* GetSystem()
    {
        return static_cast<T*>(GetSystem(T::GUID()));
    }

    void Init();
    void Cleanup();

    void Reset();

    void Load(std::string_view map);
    void ReloadMap();

    bool Update();

    void RenderScene();
    void RenderOverlay();

    GameObject* AddObject();

    void Quit() { Run = false; }

    WorldState& GetState() { return State; }
    const WorldState& GetState() const { return State; }

    Map& GetMap() { return WorldMap; }
    const Map& GetMap() const { return WorldMap; }

    Raycaster& GetRaycaster() { return WorldRaycaster; }

protected:
    std::vector<System*> PreUpdateSystems;
    std::vector<System*> UpdateSystems;
    std::vector<System*> PostUpdateSystems;
    std::vector<System*> AsyncSystems;
    std::vector<System*> PreRenderSystems;
    std::vector<System*> RenderSystems;
    std::vector<System*> PostRenderSystems;

    std::unordered_map<size_t, std::unique_ptr<System>> Systems;

    std::unique_ptr<GameObject> RootObject;

    bool Run = true;

    WorldState State = WorldState::Empty;
    Map WorldMap;
    Raycaster WorldRaycaster;

    std::string CurrentWorldMap;

protected:
    void SetupSystems();
};