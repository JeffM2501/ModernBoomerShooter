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
#include "components/mob_behavior_component.h"

#include "utilities/debug_draw_utility.h"

#include "game_object.h"
#include "scene.h"
#include "map/map.h"


SceneRenderSystem::SceneRenderSystem()
    : System()
    , Render(App::GetScene().GetMap(), App::GetScene().GetRaycaster())
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
    Render.Reset();

    PlayerManager = App::GetSystem<PlayerManagementSystem>();
    MapObjects = App::GetSystem<MapObjectSystem>();
    Mobs = App::GetSystem<MobSystem>();

    std::string skyboxName = App::GetScene().GetMap().LightInfo.SkyboxTextureName;
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

    auto& worldShader = Render.GetWorldShader();

    AnimationShaderLocation = GetShaderLocation(worldShader, "animate");
    int val = 0;
    SetShaderValue(worldShader, AnimationShaderLocation, &val, SHADER_UNIFORM_INT);

    ObjectLights.SetShader(Render.GetWorldShader());
    ObjectLights.ClearLights();

    for (auto* mapObjet : MapObjects->MapObjects.Components)
    {
        mapObjet->Instance->SetShader(Render.GetWorldShader());
    }

    for (auto* mobObject : Mobs->Mobs.Components)
    {
        auto* instance = mobObject->GetModelInstance();
        if (instance)
            instance->SetShader(Render.GetWorldShader());
    }
}

float GetFOVX(float fovY)
{
    float aspectRatio = GetScreenWidth() / (float)GetScreenHeight();

    return 2.0f * atanf(tanf(fovY * DEG2RAD * 0.5f) * aspectRatio) * RAD2DEG;
}

void SceneRenderSystem::OnUpdate()
{
    if (App::GetState() != GameState::Playing)
        return;

    for (auto& zone : App::GetScene().GetMap().LightZones)
        zone.Advance();

    if (PlayerManager)
        Render.SetViewpoint(PlayerManager->GetPlayerPos(), PlayerManager->GetPlayerPitch(), PlayerManager->GetPlayerFacing());

    App::GetScene().GetRaycaster().SetOutputSize(GetScreenWidth(), GetFOVX(Render.Viepoint.fovy));

    if (PlayerManager)
        App::GetScene().GetRaycaster().StartFrame(PlayerManager->GetPlayerPos(), PlayerManager->GetPlayerFacing());

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

    int val = 0;
    SetShaderValue(ObjectLights.GetShader(), AnimationShaderLocation, &val, SHADER_UNIFORM_INT);


    Render.Render();

    BeginMode3D(Render.Viepoint);

    for (auto* mapObjet : MapObjects->MapObjects.Components)
    {
        auto& transform = mapObjet->GetOwner()->MustGetComponent<TransformComponent>();
        mapObjet->Instance->Draw(transform);

        if (mapObjet->Solid && GlobalVars::ShowDebugDraw)
        {
            rlPushMatrix();
            rlTranslatef(transform.Position.x, transform.Position.y, transform.Position.z);
            DrawBoundingBox(mapObjet->Instance->Geometry->GetBounds(), ColorAlpha(RED, 0.75f));
            rlPopMatrix();
        }
    }
    val = 1;
    SetShaderValue(ObjectLights.GetShader(), AnimationShaderLocation, &val, SHADER_UNIFORM_INT);
    for (auto mob : Mobs->Mobs.Components)
    {
        mob->Draw();
    }

    DebugDrawUtility::Draw3D(Render.Viepoint);

    EndMode3D();
}