#include "systems/scene_render_system.h"

#include "raylib.h"
#include "rlgl.h"

#include "systems/player_management_system.h"

#include "services/resource_manager.h"
#include "services/texture_manager.h"
#include "services/table_manager.h"

#include "world.h"
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

    std::string skyboxName = WorldPtr->GetMap().LightInfo.SkyboxTextureName;
    if (skyboxName.empty())
        skyboxName = TableManager::GetTable(BootstrapTable)->GetField("default_skybox").data();

    SkyboxTexture.id = 0;

    if (!skyboxName.empty())
    {
        auto skyboxresource = ResourceManager::OpenResource(skyboxName);
        if (skyboxresource)
        {
            Image img = LoadImageFromMemory(GetFileExtension(skyboxName.c_str()), skyboxresource->DataBuffer, int(skyboxresource->DataSize));
            ResourceManager::ReleaseResource(skyboxresource);
            if (IsImageValid(img))
            {
                SkyboxTexture = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);    // CUBEMAP_LAYOUT_PANORAMA
                UnloadImage(img);
            }
        }

        if (!IsTextureValid(SkyboxTexture))
        {
            SkyboxTexture = TextureManager::GetTexture(skyboxName);
        }
    }
    
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
}

void SceneRenderSystem::OnUpdate()
{
    if (!WorldPtr || WorldPtr->GetState() != WorldState::Playing)
        return;

    if (PlayerManager)
        Render.SetViewpoint(PlayerManager->PlayerPos, PlayerManager->PlayerYaw, PlayerManager->PlayerPitch);

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

            EndMode3D();
        }
    }
    else
    {
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), DARKBLUE, SKYBLUE);
    }
    Render.Render();
}