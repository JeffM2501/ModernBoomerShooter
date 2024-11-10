#include "systems/overlay_render_system.h"
#include "services/texture_manager.h"
#include "raylib.h"

void OverlayRenderSystem::OnUpdate()
{
    DrawText(TextFormat("Used Video Memory %0.2fkb", TextureManager::GetUsedVRAM()/1024.0f), 10, GetScreenHeight()-30, 20, WHITE);
    DrawFPS(10, GetScreenHeight() - 50);
}
