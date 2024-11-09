#include "raylib.h"
#include "raymath.h"

#include "world.h"

#include "services/resource_manager.h"
#include "services/texture_manager.h"

#include "systems/input_system.h"

namespace App
{
    World GameWorld;

    bool Run = false;

    Texture TestTexture;

    Vector2  TestPos = { 0 };

    InputSystem* Input = nullptr;

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

        Input = GameWorld.GetSystem<InputSystem>();
    }

    void NewFrame()
    {
        Run = GameWorld.Update();

        if (!Run)
            return;

        TestPos.y -= Input->GetActionValue(Actions::Forward);
        TestPos.x += Input->GetActionValue(Actions::Sideways);

        BeginDrawing();
        ClearBackground(BLACK);

        GameWorld.RenderScene();

        DrawTexture(TestTexture, TestPos.x, TestPos.y, WHITE);

        DrawText(TextFormat("Used Memory %d", TextureManager::GetUsedVRAM()), 10, 600, 20, WHITE);

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