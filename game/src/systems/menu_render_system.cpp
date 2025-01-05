#include "systems/menu_render_system.h"
#include "services/global_vars.h"

#include "scene.h"
#include "raylib.h"
#include "rlgl.h"

void MenuRenderSystem::OnUpdate()
{
    switch (App::GetState())
    {
    default:
    case GameState::Empty:
        ClearBackground(DARKBLUE);
        DrawText("Empty", 600, 400, 40, WHITE);
        break;

    case GameState::Loading:
    {
        ClearBackground(BLACK);
        LoadingSpinnerAngle += 180 * GetFrameTime();

        float loadingSize = 40;
        Font font = GetFontDefault();

        Vector2 size = MeasureTextEx(font, "Loading", loadingSize, loadingSize / 10);

        Vector2 pos = { GetScreenWidth() * 0.5f - size.x * 0.5f, GetScreenHeight() * 0.5f - size.y * 0.5f };
        DrawTextEx(font, "Loading", pos, loadingSize, loadingSize/10, WHITE);

        DrawTextEx(font, "Please Wait", pos + (Vector2UnitY * loadingSize), loadingSize/2, loadingSize / 20, WHITE);

        float throberSize = 60;

        rlPushMatrix();

        rlTranslatef(pos.x - throberSize, pos.y + loadingSize * 0.5f, 0);
        
        
        rlPushMatrix(); 
        rlRotatef(LoadingSpinnerAngle, 0, 0, 1);
        DrawRectangleRec(Rectangle{ -throberSize * 0.5f, -throberSize * 0.5f, throberSize, throberSize }, WHITE);
        rlPopMatrix();

        rlPushMatrix();
        rlRotatef(-LoadingSpinnerAngle - 45, 0, 0, 1);
        DrawRectangleRec(Rectangle{ -throberSize * 0.25f, -throberSize * 0.25f, throberSize * 0.5f, throberSize * 0.5f }, DARKGRAY);
        rlPopMatrix();

        rlPopMatrix();
    }
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
