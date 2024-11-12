#include "raylib.h"
#include "raymath.h"

#include "world.h"

#include "services/resource_manager.h"
#include "services/texture_manager.h"
#include "services/table_manager.h"
#include "services/game_time.h"

#include "systems/input_system.h"

namespace App
{
    World GameWorld;

    bool Run = false;

    void Init()
    {
        Run = true;
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(1280, 800, "ModernBoomerShooter");
        SetTargetFPS(144);

        GameTime::ComputeNominalFPS();
        ResourceManager::Init("resources");
        TextureManager::Init();
        GameWorld.Init();

        auto* table = TableManager::GetTable(BootstrapTable);

        if (!table)
        {
            Run = false;
            TraceLog(LOG_FATAL, "Unable to locate bootstrap table at %s, exiting", BootstrapTable);
        }

        GameWorld.Load(table->GetField("boot_level"));
    }

    void NewFrame()
    {
        Run = GameWorld.Update();

        if (!Run)
            return;

        BeginDrawing();
        ClearBackground(BLACK);

        GameWorld.RenderScene();
        GameWorld.RenderOverlay();
        EndDrawing();
    }

    void Cleanup()
    {
        GameWorld.Cleanup();
        TextureManager::Cleanup();
        ResourceManager::Cleanup();
        TableManager::Cleanup();
        CloseWindow();
    }

    bool Quit()
    {
        return !Run;
    }
}
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