#include "systems/scene_render_system.h"
#include "systems/player_management_system.h"
#include "world.h"
#include "raylib.h"
#include "map/map.h"


SceneRenderSystem::SceneRenderSystem(World* world)
    : System(world)
    , Render(world->GetMap())
{
    Render.Reset();
}

void SceneRenderSystem::OnSetup()
{
    if (!WorldPtr)
        return;

    PlayerManager = WorldPtr->GetSystem<PlayerManagementSystem>();
}

void SceneRenderSystem::OnUpdate()
{
    if (!WorldPtr || WorldPtr->GetState() != WorldState::Playing)
        return;

    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), SKYBLUE, WHITE);

    //todo, render skybox, setup fog shader, and other stuff


    if (PlayerManager)
        Render.SetViewpoint(PlayerManager->PlayerPos, PlayerManager->PlayerYaw, PlayerManager->PlayerPitch);

    Render.Render();
}