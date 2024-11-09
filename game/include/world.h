#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include "game_object.h"
#include "system.h"

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

    bool Update();

    void RenderScene();
    void RenderOverlay();

    GameObject* AddObject();

    void Quit() { Run = false; }

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

protected:
    void SetupSystems();
};