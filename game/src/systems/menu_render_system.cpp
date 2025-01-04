#include "systems/menu_render_system.h"
#include "services/global_vars.h"

#include "scene.h"
#include "raylib.h"

void MenuRenderSystem::OnUpdate()
{
    switch (App::GetState())
    {
    default:
    case GameState::Empty:
        DrawText("Empty", 600, 400, 40, WHITE);
        break;

    case GameState::Loading:
        DrawText("Loading", 600, 400, 40, WHITE);
        break;
       
    case GameState::Playing:

        if (InMenu)
        {
            DrawText("Paused, Y to exit, escape to resume", 300, 400, 40, WHITE);
            if (IsKeyPressed(KEY_Y))
            {
                App::Quit();
            }
        }

        if (IsKeyPressed(KEY_ESCAPE))
        {
            InMenu = !InMenu;
            GlobalVars::Paused = InMenu;
        }

        break;

    case GameState::Closing:
        DrawText("Closing", 600, 400, 40, WHITE);
        break;
    }

    bool wantCuror = InMenu || GlobalVars::UseMouseDrag;

    if (wantCuror && IsCursorHidden())
        EnableCursor();

    if (!wantCuror && !IsCursorHidden())
        DisableCursor();
}
