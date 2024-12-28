#pragma once

#include "system.h"
#include "map/map_render.h"
#include "utilities/lighting_system.h"
#include "services/model_manager.h"
#include "raylib.h"

class PlayerManagementSystem;
class MapObjectSystem;
class MobSystem;

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
    PlayerManagementSystem* PlayerManager = nullptr;
    MapObjectSystem* MapObjects = nullptr;
    MobSystem* Mobs = nullptr;

    Shader SkyboxShader = { 0 };
    Texture2D SkyboxTexture = { 0 };
    Mesh Skybox = { 0 };
    Material SkyboxMaterial = { 0 };

    LightScene ObjectLights;
    Shader MapShader = { 0 };

    int AnimationShaderLocation = 0;
};