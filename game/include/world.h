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
        RegisterSystem(stage, std::move(std::make_unique<T>()));
    }

    void Init();
    void Cleanup();

    bool Update();

   void RenderScene();
   void RenderOverlay();

    GameObject* AddObject();

protected:
    std::vector<System*> PreUpdateSystems;
    std::vector<System*> UpdateSystems;
    std::vector<System*> PostUpdateSystems;
    std::vector<System*> AsyncSystems;
    std::vector<System*> PreRenderSystems;
    std::vector<System*> RenderSystems;
    std::vector<System*> PostRenderSystems;

    std::unordered_map<uint64_t, std::unique_ptr<System>> Systems;

    std::unique_ptr<GameObject> RootObject;

protected:
    void SetupSystems();
};