#include "raylib.h"
#include "raymath.h"

#include "world.h"

#include "services/resource_manager.h"
#include "services/texture_manager.h"

namespace App
{
    World GameWorld;

    bool Run = false;

    Texture TestTexture;

    void Init()
    {
        Run = true;
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(1280, 800, "ModernBoomerShooter");
        SetTargetFPS(144);

        ResourceManager::Init("resources");
        TextureManager::Init();

        TestTexture = TextureManager::GetTexture("test.png");

        GameWorld.Init();
    }

    void NewFrame()
    {
        Run = GameWorld.Update();

        if (!Run)
            return;

        BeginDrawing();
        ClearBackground(BLACK);

        GameWorld.RenderScene();

        DrawTexture(TestTexture, 0, 0, WHITE);

        DrawText("Hello Raylib!", 10, 10, 20, WHITE);

        GameWorld.RenderOverlay();
        EndDrawing();
    }

    void Cleanup()
    {
        GameWorld.Cleanup();
        TextureManager::Cleanup();
        ResourceManager::Cleanup();
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