#include "raylib.h"
#include "raymath.h"

#include "world.h"

#include "services/global_vars.h"
#include "services/resource_manager.h"
#include "services/texture_manager.h"
#include "services/table_manager.h"
#include "services/game_time.h"
#include "services/model_manager.h"

#include "systems/input_system.h"

namespace App
{
    // global world
    World GameWorld;

    // application running state
    bool Run = false;


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

        // initialize the GPU shared resource managers
        TextureManager::Init();
        ModelManager::Init();

        // Setup all systems
        GameWorld.Init();

        // load the bootstrap table, all game data runs from this
        auto* table = TableManager::GetTable(BootstrapTable);

        if (!table)
        {
            Run = false;
            TraceLog(LOG_FATAL, "Unable to locate bootstrap table at %s, exiting", BootstrapTable);
        }

        // find the boot level
        GameWorld.Load(table->GetField("boot_level"));
    }

    void NewFrame()
    {
        // have all systems update
        Run = GameWorld.Update();

        // bail out if we want to die
        if (!Run)
            return;

        // Render
        BeginDrawing();
        ClearBackground(MAGENTA); // garish color so we can see if any gaps.
        GameWorld.RenderScene(); // the 3d part
        GameWorld.RenderOverlay(); // the 2d part
        EndDrawing();
    }

    void Cleanup()
    {
        GameWorld.Cleanup();
        TextureManager::Cleanup();
        ResourceManager::Cleanup();
        TableManager::Cleanup();
        ModelManager::Cleanup();
        CloseWindow();
    }

    bool Quit()
    {
        return !Run;
    }
}

// simple main app
int main()
{
    App::Init();

    while (!App::Quit())
    {
        App::NewFrame();
    }
    App::Cleanup();

    return 0;
}