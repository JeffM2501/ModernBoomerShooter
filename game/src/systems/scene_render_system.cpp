#include "systems/scene_render_system.h"

#include "raylib.h"
#include "rlgl.h"

#include "systems/player_management_system.h"
#include "systems/map_object_system.h"

#include "services/resource_manager.h"
#include "services/texture_manager.h"
#include "services/table_manager.h"
#include "services/global_vars.h"

#include "components/transform_component.h"
#include "components/map_object_component.h"
#include "components/trigger_component.h"

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

    object->Instance->SetShader(ObjectLights.GetShader());
}

void SceneRenderSystem::OnSetup()
{
    if (!WorldPtr)
        return;

    Render.Reset();

    PlayerManager = WorldPtr->GetSystem<PlayerManagementSystem>();
    MapObjects = WorldPtr->GetSystem<MapObjectSystem>();

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

    ObjectLights.SetShader(TextureManager::GetShader("object"));
    ObjectLights.ClearLights();

    float ambientScale = 0.5f;
    ObjectLights.SetAmbientColor(WorldPtr->GetMap().LightInfo.InteriorAmbientLevel * ambientScale);

    auto * light = static_cast<DirectionalLight*>(ObjectLights.AddLight(LightTypes::Directional));

    float ambientAngle = WorldPtr->GetMap().LightInfo.AmbientAngle;// -90;

    Vector3 lightVec = {
        sinf(DEG2RAD * ambientAngle),
        cosf(DEG2RAD * ambientAngle),
        -1.0f
    };

    light->SetDirection(Vector3Normalize(lightVec));

    for (auto* mapObjet : MapObjects->MapObjects)
    {
        mapObjet->Instance->SetShader(ObjectLights.GetShader());
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
        for (auto* trigger : MapObjects->Triggers)
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

    Render.Render();

    BeginMode3D(Render.Viepoint);
    // draw objects
    ObjectLights.ApplyLights(Render.Viepoint);

    for (auto* mapObjet : MapObjects->MapObjects)
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

    DrawDebugShapes();
    EndMode3D();
}