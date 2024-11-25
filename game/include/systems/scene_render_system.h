#pragma once

#include "system.h"
#include "map/map_render.h"
#include "utilities/lighting_system.h"
#include "services/model_manager.h"
#include "raylib.h"

class SceneRenderSystem : public System
{
public:
    DEFINE_SYSTM_NO_CONSTRUCTOR(SceneRenderSystem);

    SceneRenderSystem(World* world);
    void MapObjectAdded(class MapObjectComponent* object);

protected:
    void OnSetup() override;
    void OnUpdate() override;

protected:
    MapRenderer Render;
    class PlayerManagementSystem* PlayerManager = nullptr;
    class MapObjectSystem* MapObjects = nullptr;

    Shader SkyboxShader = { 0 };
    Texture2D SkyboxTexture = { 0 };
    Mesh Skybox = { 0 };
    Material SkyboxMaterial = { 0 };

    LightScene ObjectLights;
};