#include "world.h"

#include "system.h"
#include "game_object.h"

// systems
#include "systems/input_system.h"
#include "systems/scene_render_system.h"
#include "systems/overlay_render_system.h"
#include "systems/console_render_system.h"
#include "systems/menu_render_system.h"
#include "systems/player_management_system.h"

#include "map/map_reader.h"

World::World()
{
    RootObject = std::make_unique<GameObject>(this);
    SetupSystems();
}

void World::RegisterSystem(SystemStage stage, std::unique_ptr<System> system)
{
    if (Systems.find(system->GetGUID()) != Systems.end())
        return;

    switch (stage)
    {
    case SystemStage::PreUpdate:
        PreUpdateSystems.push_back(system.get());
        break;
    case SystemStage::Update:
        UpdateSystems.push_back(system.get());
        break;
    case SystemStage::PostUpdate:
        PostUpdateSystems.push_back(system.get());
        break;
    case SystemStage::Async:
        AsyncSystems.push_back(system.get());
        break;
    case SystemStage::PreRender:
        PreRenderSystems.push_back(system.get());
        break;
    case SystemStage::Render:
        RenderSystems.push_back(system.get());
        break;
    case SystemStage::PostRender:
        PostRenderSystems.push_back(system.get());
        break;
    }

    Systems.insert_or_assign(system->GetGUID(), std::move(system));
}

System* World::GetSystem(size_t systemId)
{
    auto itr = Systems.find(systemId);
    if (itr == Systems.end())
        return nullptr;

    return itr->second.get();
}

void World::Init()
{
    WorldRaycaster.SetMap(&WorldMap);

    for (auto& [id, system] : Systems)
    {
        system->Init();
    }
}

void World::Load(std::string_view map)
{
    WorldMap.Clear();

    if (!map.empty())
        ReadWorldTMX(map.data(), *this);

    WorldRaycaster.SetMap(&WorldMap);

    Reset();
}

void World::Reset()
{
    for (auto& [id, system] : Systems)
    {
        system->Setup();
    }
}

void World::Cleanup()
{
    for (auto& [id, system] : Systems)
    {
        system->Cleanup();
    }
    PreUpdateSystems.clear();
    UpdateSystems.clear();
    PostUpdateSystems.clear();
    AsyncSystems.clear();
    PreRenderSystems.clear();
    RenderSystems.clear();
    PostRenderSystems.clear();
    Systems.clear();
}

bool World::Update()
{
    for (auto& system : PreUpdateSystems)
        system->Update();

    for (auto& system : UpdateSystems)
        system->Update();

    for (auto& system : PostUpdateSystems)
        system->Update();

    // async don't get an update

    return Run;
}

void World::RenderScene()
{
    for (auto& system : PreRenderSystems)
        system->Update();

    for (auto& system : RenderSystems)
        system->Update();
}

void World::RenderOverlay()
{
    for (auto& system : PostRenderSystems)
        system->Update();
}

void World::SetupSystems()
{
    // register standard systems
    RegisterSystem<InputSystem>(SystemStage::PreUpdate);

    RegisterSystem<PlayerManagementSystem>(SystemStage::Update);

    RegisterSystem<SceneRenderSystem>(SystemStage::Render);

    RegisterSystem<OverlayRenderSystem>(SystemStage::PostRender);
    RegisterSystem<MenuRenderSystem>(SystemStage::PostRender); 
    RegisterSystem<ConsoleRenderSystem>(SystemStage::PostRender);
}

GameObject* World::AddObject()
{
    auto* object = RootObject->AddChild();

    return object;
}
