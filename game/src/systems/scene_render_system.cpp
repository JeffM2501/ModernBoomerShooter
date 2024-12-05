#include "systems/scene_render_system.h"

#include "raylib.h"
#include "rlgl.h"

#include "systems/player_management_system.h"
#include "systems/map_object_system.h"
#include "systems/mobile_object_system.h"

#include "services/resource_manager.h"
#include "services/texture_manager.h"
#include "services/table_manager.h"
#include "services/global_vars.h"

#include "components/transform_component.h"
#include "components/map_object_component.h"
#include "components/trigger_component.h"
#include "components/mobile_object_component.h"

#include "game_object.h"
#include "world.h"
#include "map/map.h"


SceneRenderSystem::SceneRenderSystem(World* world)
    : System(world)
    , Render(world->GetMap(), world->GetRaycaster())
{
}

void SceneRenderSystem::MapObjectAdded(class MapObjectComponent* object)
{
    if (!MapObjects)
        return;

    object->Instance->SetShader(Render.GetWorldShader());
}

void SceneRenderSystem::OnSetup()
{
    if (!WorldPtr)
        return;

    Render.Reset();

    PlayerManager = WorldPtr->GetSystem<PlayerManagementSystem>();
    MapObjects = WorldPtr->GetSystem<MapObjectSystem>();
    Mobs = WorldPtr->GetSystem<MobSystem>();

    std::string skyboxName = WorldPtr->GetMap().LightInfo.SkyboxTextureName;
    if (skyboxName.empty())
        skyboxName = TableManager::GetTable(BootstrapTable)->GetField("default_skybox").data();

    SkyboxTexture.id = 0;

    if (!skyboxName.empty())
        SkyboxTexture = TextureManager::GetTextureCubemap(skyboxName);
    
    SkyboxShader = TextureManager::GetShader("skybox");

    int value = 0;
    SetShaderValue(SkyboxShader, GetShaderLocation(SkyboxShader, "doGamma" ), &value, SHADER_UNIFORM_INT);
    SetShaderValue(SkyboxShader, GetShaderLocation(SkyboxShader, "vflipped"), &value, SHADER_UNIFORM_INT);

    value = MATERIAL_MAP_CUBEMAP;
    SetShaderValue(SkyboxShader, GetShaderLocation(SkyboxShader, "environmentMap"), &value, SHADER_UNIFORM_INT);

    Skybox = GenMeshCube(-10,-10,-10);
    SkyboxMaterial = LoadMaterialDefault();
    SkyboxMaterial.maps[MATERIAL_MAP_CUBEMAP].texture = SkyboxTexture;
    SkyboxMaterial.shader = SkyboxShader;

    ObjectLights.SetShader(Render.GetWorldShader());
    ObjectLights.ClearLights();

    for (auto* mapObjet : MapObjects->MapObjects.Components)
    {
        mapObjet->Instance->SetShader(Render.GetWorldShader());
    }
}

float GetFOVX(float fovY)
{
    float aspectRatio = GetScreenWidth() / (float)GetScreenHeight();

    return 2.0f * atanf(tanf(fovY * DEG2RAD * 0.5f) * aspectRatio) * RAD2DEG;
}

static constexpr float DebugFloorHeight = 0.01f;
void SceneRenderSystem::DrawDebugShapes()
{
    if (GlobalVars::ShowTriggerVolumes)
    {
        for (auto* trigger : MapObjects->Triggers.Components)
        {
            DrawLine3D(Vector3{ trigger->Bounds.x, trigger->Bounds.y, DebugFloorHeight },
                Vector3{ trigger->Bounds.x + trigger->Bounds.width, trigger->Bounds.y, DebugFloorHeight },
                RED);

            DrawLine3D(Vector3{ trigger->Bounds.x + trigger->Bounds.width, trigger->Bounds.y, DebugFloorHeight },
                Vector3{ trigger->Bounds.x + trigger->Bounds.width, trigger->Bounds.y + trigger->Bounds.height, DebugFloorHeight },
                RED);

            DrawLine3D(Vector3{ trigger->Bounds.x + trigger->Bounds.width, trigger->Bounds.y + trigger->Bounds.height, DebugFloorHeight },
                Vector3{ trigger->Bounds.x, trigger->Bounds.y + trigger->Bounds.height, DebugFloorHeight },
                RED);

            DrawLine3D(Vector3{ trigger->Bounds.x, trigger->Bounds.y + trigger->Bounds.height, DebugFloorHeight },
                Vector3{ trigger->Bounds.x, trigger->Bounds.y, DebugFloorHeight },
                RED);
        }
    }
}

void SceneRenderSystem::OnUpdate()
{
    if (!WorldPtr || WorldPtr->GetState() != WorldState::Playing)
        return;

    for (auto& zone : WorldPtr->GetMap().LightZones)
        zone.Advance();

    if (PlayerManager)
        Render.SetViewpoint(PlayerManager->GetPlayerPos(), PlayerManager->GetPlayerYaw(), PlayerManager->GetPlayerPitch());

    WorldPtr->GetRaycaster().SetOutputSize(GetScreenWidth(), GetFOVX(Render.Viepoint.fovy));

    if (PlayerManager)
        WorldPtr->GetRaycaster().StartFrame(PlayerManager->GetPlayerPos(), PlayerManager->GetPlayerFacing());

    if (IsTextureValid(SkyboxTexture))
    {
        BeginMode3D(Render.Viepoint);
        {
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
            Matrix mat = MatrixRotate(Vector3UnitX, -90 * DEG2RAD);
            DrawMesh(Skybox, SkyboxMaterial, mat);
            rlEnableDepthMask();
            rlEnableBackfaceCulling();
        }
        EndMode3D();
    }
    else
    {
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), DARKBLUE, SKYBLUE);
    }

    ObjectLights.ApplyLights(Render.Viepoint);

    Render.Render();

    BeginMode3D(Render.Viepoint);

    for (auto* mapObjet : MapObjects->MapObjects.Components)
    {
        auto& transform = mapObjet->GetOwner()->MustGetComponent<TransformComponent>();
        mapObjet->Instance->Draw(transform);

        if (mapObjet->Solid && GlobalVars::ShowCollisionVolumes)
        {
            rlPushMatrix();
            rlTranslatef(transform.Position.x, transform.Position.y, transform.Position.z);
            DrawBoundingBox(mapObjet->Instance->Geometry->GetBounds(), ColorAlpha(RED, 0.75f));
            rlPopMatrix();
        }
    }
    
    BeginShaderMode(ObjectLights.GetShader());
    for (auto mob : Mobs->Mobs.Components)
    {
        auto* transform = mob->GetOwner()->GetComponent<TransformComponent>();
        if (!transform)
            continue;

        rlPushMatrix();
        rlTranslatef(transform->Position.x, transform->Position.y, transform->Position.z + 0.375f);
        rlRotatef(transform->Facing, 0,0,1);
        DrawCube(Vector3Zeros, 0.25f, 0.25f, 0.75f, RED);
        DrawCube(Vector3UnitY * 0.125f + Vector3UnitZ * 0.3f, 0.25f, 0.005f, 0.125f, YELLOW);
        DrawCube(Vector3UnitZ * 0.125f, 0.5f, 0.25f, 0.125f, RED);
        DrawCube(Vector3UnitX * 0.3f + Vector3UnitY * 0.125f, 0.125f, 0.4f, 0.125f, PURPLE);
        rlPopMatrix();
    }

    EndShaderMode();

    DrawDebugShapes();
    EndMode3D();
}