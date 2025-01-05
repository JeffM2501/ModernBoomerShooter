#include "systems/overlay_render_system.h"
#include "systems/player_management_system.h"
#include "services/texture_manager.h"
#include "services/global_vars.h"
#include "scene.h"

#include "raylib.h"

void OverlayRenderSystem::OnUpdate()
{
    if (App::GetState() != GameState::Playing)
        return;

    DrawText(TextFormat("Rays Cast %d", App::GetScene().GetRaycaster().GetCastCount()), 10, GetScreenHeight() - 50, 20, SKYBLUE);
    DrawText(TextFormat("Cells Drawn %d of %d total cells", GlobalVars::UseVisCulling ? App::GetScene().GetRaycaster().GetHitCelList().size() : App::GetScene().GetMap().Cells.size(), App::GetScene().GetMap().Cells.size()), 10, GetScreenHeight() - 70, 20, YELLOW);

    float vram = TextureManager::GetUsedVRAM() / 1024.0f;
    const char* vramSuffix = "kb";
    if (vram > 1024)
    {
        vram /= 1024.0f;
        vramSuffix = "mb";
    }
    if (vram > 1024)
    {
        vram /= 1024.0f;
        vramSuffix = "gb";
    }

    DrawText(TextFormat("Used Texture Memory %0.2f%s", vram, vramSuffix), 10, GetScreenHeight()-30, 20, WHITE);
    DrawFPS(10, GetScreenHeight() - 90);

    if (GlobalVars::ShowCoordinates)
    {
        auto playerPos = App::GetSystem<PlayerManagementSystem>()->GetPlayerPos();

        DrawText(TextFormat("Player X%0.2f Y%0.2f", playerPos.x, playerPos.y), GetScreenWidth()-250, GetScreenHeight() - 30, 20, WHITE);
    }
}
