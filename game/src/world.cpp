#include "world.h"

#include "system.h"
#include "game_object.h"

// systems
#include "systems/audio_system.h"
#include "systems/console_render_system.h"
#include "systems/input_system.h"
#include "systems/map_object_system.h"
#include "systems/menu_render_system.h"
#include "systems/overlay_render_system.h"
#include "systems/player_management_system.h"
#include "systems/scene_render_system.h"
#include "systems/mobile_object_system.h"

// services
#include "services/global_vars.h"

#include "map/map_reader.h"

static std::hash<std::string_view> StringHasher;

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
    GlobalVars::Paused = false;

    WorldRaycaster.SetMap(&WorldMap);

    for (auto& [id, system] : Systems)
    {
        system->Init();
    }
}

void World::Load(std::string_view map)
{
    CurrentWorldMap = map;
    WorldMap.Clear();

    if (!map.empty())
        ReadWorldTMX(map.data(), *this);

    WorldRaycaster.SetMap(&WorldMap);

    Reset();
}

void World::ReloadMap()
{
    WorldMap.Clear();
    if (!CurrentWorldMap.empty())
        ReadWorldTMX(CurrentWorldMap.data(), *this);

    WorldRaycaster.SetMap(&WorldMap);
}

void World::Reset()
{
    GlobalVars::Paused = false;

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

    GlobalVars::Paused = true;
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
    RegisterSystem<MapObjectSystem>(SystemStage::PreUpdate);

    RegisterSystem<MobSystem>(SystemStage::Update);
    RegisterSystem<PlayerManagementSystem>(SystemStage::Update);

    RegisterSystem<AudioSystem>(SystemStage::PostUpdate);

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


void World::AddEventHandler(size_t hash, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token)
{
    auto itr = EventHandlers.find(hash);
    if (itr == EventHandlers.end())
    {
        itr = EventHandlers.insert_or_assign(hash, std::vector<GameObjectEventRecord>()).first;
    }

    itr->second.emplace_back(GameObjectEventRecord{ handler, token });
}

void World::AddEventHandler(std::string_view name, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token)
{
    AddEventHandler(StringHasher(name), handler, token);
}

void World::CallEvent(size_t hash, GameObject* sender, GameObject* target)
{
    auto itr = EventHandlers.find(hash);
    if (itr == EventHandlers.end())
        return;

    for (std::vector<GameObjectEventRecord>::iterator eventItr = itr->second.begin(); eventItr != itr->second.end();)
    {
        if (eventItr->LifetimeToken->IsValid())
        {
            eventItr->Handler(hash, sender, target);
            eventItr++;
        }
        else
        {
            eventItr = itr->second.erase(eventItr);
        }
    }
}
