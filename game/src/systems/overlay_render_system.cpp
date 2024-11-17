#include "systems/overlay_render_system.h"
#include "services/texture_manager.h"
#include "services/global_vars.h"
#include "world.h"

#include "raylib.h"

void OverlayRenderSystem::OnUpdate()
{
    DrawText(TextFormat("Rays Cast %d", WorldPtr->GetRaycaster().GetCastCount()), 10, GetScreenHeight() - 50, 20, SKYBLUE);
    DrawText(TextFormat("Cells Drawn %d of %d total cells", GlobalVars::UseVisCulling ? WorldPtr->GetRaycaster().GetHitCelList().size() : WorldPtr->GetMap().Cells.size(), WorldPtr->GetMap().Cells.size()), 10, GetScreenHeight() - 70, 20, YELLOW);
    DrawText(TextFormat("Used Texture Memory %0.2fkb", TextureManager::GetUsedVRAM()/1024.0f), 10, GetScreenHeight()-30, 20, WHITE);
    DrawFPS(10, GetScreenHeight() - 90);
}
