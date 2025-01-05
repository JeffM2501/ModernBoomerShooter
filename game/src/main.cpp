#include "raylib.h"
#include "raymath.h"

#include "game.h"
#include "scene.h"

// services
#include "services/global_vars.h"
#include "services/resource_manager.h"
#include "services/texture_manager.h"
#include "services/table_manager.h"
#include "services/game_time.h"
#include "services/model_manager.h"
#include "services/character_manager.h"

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

namespace App
{
    // global world
    Scene GameWorld;

    // application running state
    bool Run = false;

    std::vector<System*> PreUpdateSystems;
    std::vector<System*> UpdateSystems;
    std::vector<System*> PostUpdateSystems;
    std::vector<System*> AsyncSystems;
    std::vector<System*> PreRenderSystems;
    std::vector<System*> RenderSystems;
    std::vector<System*> PostRenderSystems;

    std::unordered_map<size_t, std::unique_ptr<System>> Systems;

    std::unordered_map<size_t, std::vector<GameObjectEventRecord>> EventHandlers;

    static std::hash<std::string_view> StringHasher;

    GameState AppState = GameState::Empty;

    GameState& GetState() { return AppState; }

    void SetupSystems()
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

    void RegisterSystem(SystemStage stage, std::unique_ptr<System> system)
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

    System* GetSystem(size_t systemId)
    {
        auto itr = Systems.find(systemId);
        if (itr == Systems.end())
            return nullptr;

        return itr->second.get();
    }

    void Reset()
    {
        GlobalVars::Paused = false;

        for (auto& [id, system] : Systems)
        {
            system->Setup();
        }
    }

    Scene& GetScene()
    {
        return GameWorld;
    }

    // Setup raylib and all systems and services
    void Init()
    {
        Run = true;

        uint32_t flags = FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI;
        if (GlobalVars::UseVSync)
            flags |= FLAG_VSYNC_HINT;

        SetConfigFlags(flags);

        InitWindow(1280, 800, "ModernBoomerShooter");
        SetExitKey(KEY_NULL);
        SetTargetFPS(GlobalVars::FPSCap);
        
        // setup debug FPS limit
        GameTime::ComputeNominalFPS();
        
        // tell the resource manager where the game resources are
        ResourceManager::Init("resources");

        // Setup all systems
        SetupSystems();

        for (auto& [id, system] : Systems)
        {
            system->Init();
        }

        AppState = GameState::Loading;
    }

    void NewFrame()
    {
        if (AppState == GameState::Loading)
        {
            bool ready = true;
            for (auto& [id, system] : Systems)
            {
                if (!system->IsReady())
                {
                    ready = false;
                }
            }

            if (ready)
            {
                // load the bootstrap table, all game data runs from this
                auto* table = TableManager::GetTable(BootstrapTable);
 
                if (!table)
                {
                    Run = false;
                    TraceLog(LOG_FATAL, "Unable to locate bootstrap table at %s, exiting", BootstrapTable);
                    return;
                }
 
                // initialize the GPU shared resource managers
                TextureManager::Init();
                ModelManager::Init();
                CharacterManager::Init();

                // setup scene
                GameWorld.Init();

                GameWorld.Load(table->GetField("boot_level"));
                
                for (auto& [id, system] : Systems)
                {
                    system->Setup();
                }

                AppState = GameState::Playing;
                GlobalVars::Paused = false;
            }
        }

        // have all systems update
        for (auto& system : PreUpdateSystems)
            system->Update();

        for (auto& system : UpdateSystems)
            system->Update();

        for (auto& system : PostUpdateSystems)
            system->Update();

        // bail out if we want to die
        if (!Run)
            return;

        // Render
        BeginDrawing();
        ClearBackground(MAGENTA); // garish color so we can see if any gaps.
        for (auto& system : PreRenderSystems)
            system->Update();

        for (auto& system : RenderSystems)
            system->Update();

        for (auto& system : PostRenderSystems)
            system->Update();

        EndDrawing();
    }

    void Cleanup()
    {
        GameWorld.Cleanup();

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

        TextureManager::Cleanup();
        ResourceManager::Cleanup();
        TableManager::Cleanup();
        ModelManager::Cleanup();
        CloseWindow();
    }

    bool WantQuit()
    {
        return !Run;
    }

    void Quit()
    {
        Run = false;
    }

    void AddEventHandler(size_t hash, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token)
    {
        auto itr = EventHandlers.find(hash);
        if (itr == EventHandlers.end())
        {
            itr = EventHandlers.insert_or_assign(hash, std::vector<GameObjectEventRecord>()).first;
        }

        itr->second.emplace_back(GameObjectEventRecord{ handler, token });
    }

    void AddEventHandler(std::string_view name, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token)
    {
        AddEventHandler(StringHasher(name), handler, token);
    }

    void CallEvent(size_t hash, GameObject* sender, GameObject* target)
    {
        auto itr = EventHandlers.find(hash);
        if (itr == EventHandlers.end())
            return;

        for (std::vector<GameObjectEventRecord>::iterator eventItr = itr->second.begin(); eventItr != itr->second.end();)
        {
            if (eventItr->LifetimeToken->IsValid())
            {
                eventItr->Handler(hash, sender, target);
                ++eventItr;
            }
            else
            {
                eventItr = itr->second.erase(eventItr);
            }
        }
    }

}

// simple main app
int main()
{
    App::Init();

    while (!App::WantQuit())
    {
        App::NewFrame();
    }
    App::Cleanup();

    return 0;
}