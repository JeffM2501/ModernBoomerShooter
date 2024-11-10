#include "systems/menu_render_system.h"
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
        // Playing
        break;

    case WorldState::Closing:
        DrawText("Closing", 600, 400, 40, WHITE);
        break;
    }
}
