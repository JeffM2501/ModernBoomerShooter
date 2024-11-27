#include "systems/menu_render_system.h"
#include "services/global_vars.h"

#include "world.h"
#include "raylib.h"

void MenuRenderSystem::OnUpdate()
{
    if (!WorldPtr)
        return;

    switch (WorldPtr->GetState())
    {
    default:
    case WorldState::Empty:
        DrawText("Empty", 600, 400, 40, WHITE);
        break;

    case WorldState::Loading:
        DrawText("Loading", 600, 400, 40, WHITE);
        break;
       
    case WorldState::Playing:

        if (InMenu)
        {
            DrawText("Paused, Y to exit, escape to resume", 300, 400, 40, WHITE);
            if (IsKeyPressed(KEY_Y))
            {
                WorldPtr->Quit();
            }
        }

        if (IsKeyPressed(KEY_ESCAPE))
            InMenu = !InMenu;

        break;

    case WorldState::Closing:
        DrawText("Closing", 600, 400, 40, WHITE);
        break;
    }

    bool wantCuror = InMenu || GlobalVars::UseMouseDrag;

    if (wantCuror && IsCursorHidden())
        EnableCursor();

    if (!wantCuror && !IsCursorHidden())
        DisableCursor();
}
