#include "utilities/lighting_system.h"
#include "raylib.h"

static constexpr char ViewPosName[] = "viewPos";
static constexpr char EnabledName[] = "enabled";
static constexpr char TypeName[] = "type";
static constexpr char PositionName[] = "position";
static constexpr char DirectionName[] = "direction";
static constexpr char ColorName[] = "color";
static constexpr char AttenuationName[] = "attenuation";
static constexpr char FallofName[] = "falloff";
static constexpr char ConeName[] = "cone";
static constexpr char AmbientName[] = "ambient";

static float ColorScale = 1.0f / 255.0f;

static float Ambient[4] = { 0.05f ,0.05f, 0.05f, 1.0f };

void LightScene::SetShader(Shader shader)
{
    LightShader.LightShader = shader;

    LightShader.LightShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, ViewPosName);
    LightShader.AmbientLoc = GetShaderLocation(shader, AmbientName);

    LightShader.UniformLocations.clear();

    std::string fieldNameCache;

    LightShader.UniformLocations.resize(MaxShaderLights);

    for (int i = 0; i < MaxShaderLights; i++)
    {
        fieldNameCache = "lights[" + std::to_string(i) + "]." + EnabledName;
        LightShader.UniformLocations[i].EnabledLoc  = GetShaderLocation(shader, fieldNameCache.c_str());

        fieldNameCache = "lights[" + std::to_string(i) + "]." + TypeName;
        LightShader.UniformLocations[i].TypeLoc = GetShaderLocation(shader, fieldNameCache.c_str());

        fieldNameCache = "lights[" + std::to_string(i) + "]." + PositionName;
        LightShader.UniformLocations[i].PositionLoc = GetShaderLocation(shader, fieldNameCache.c_str());

         fieldNameCache = "lights[" + std::to_string(i) + "]." + DirectionName;
        LightShader.UniformLocations[i].DirectionLoc = GetShaderLocation(shader, fieldNameCache.c_str());

        fieldNameCache = "lights[" + std::to_string(i) + "]." + ColorName;
        LightShader.UniformLocations[i].ColorLoc = GetShaderLocation(shader, fieldNameCache.c_str());

        fieldNameCache = "lights[" + std::to_string(i) + "]." + AttenuationName;
        LightShader.UniformLocations[i].AttenuationLoc = GetShaderLocation(shader, fieldNameCache.c_str());

        fieldNameCache = "lights[" + std::to_string(i) + "]." + FallofName;
        LightShader.UniformLocations[i].FalloffLoc = GetShaderLocation(shader, fieldNameCache.c_str());

        fieldNameCache = "lights[" + std::to_string(i) + "]." + ConeName;
        LightShader.UniformLocations[i].ConeLoc = GetShaderLocation(shader, fieldNameCache.c_str());
    }
}

Shader LightScene::GetShader()
{
    return LightShader.LightShader;
}

Light* LightScene::AddLight(LightTypes lightType)
{
    std::unique_ptr<Light> newLight;

    switch (lightType)
    {
    default:
        return nullptr;

    case LightTypes::Point:
        newLight = std::make_unique<PointLight>();
        break;

    case LightTypes::Directional:
        newLight = std::make_unique<DirectionalLight>();
        break;

    case LightTypes::Spot:
        newLight = std::make_unique<SpotLight>();
        break;
    }

    Light* light = newLight.get();
    LightList.emplace_back(std::move(newLight));
    return light;
}

void LightScene::RemoveLight(Light* light)
{
    for (auto itr = LightList.begin(); itr != LightList.end(); itr++)
    {
        if (itr->get() == light)
        {
            LightList.erase(itr);
            break;
        }
    }
}

void LightScene::ClearLights()
{
    LightList.clear();
}

void LightScene::SetAmbientColor(Color color)
{
    Ambient[0] = color.r * ColorScale;
    Ambient[1] = color.g * ColorScale;
    Ambient[2] = color.b * ColorScale;
}

void LightScene::SetAmbientColor(float luminance)
{
    Ambient[0] = luminance;
    Ambient[1] = luminance;
    Ambient[2] = luminance;
}

void LightScene::ApplyLights(const Camera3D& viewportCamera)
{
    if (!IsShaderValid(LightShader.LightShader))
        return;
    SetShaderValue(LightShader.LightShader, LightShader.AmbientLoc, Ambient, SHADER_UNIFORM_VEC4);
    SetShaderValue(LightShader.LightShader, LightShader.LightShader.locs[SHADER_LOC_VECTOR_VIEW], &viewportCamera.position, SHADER_UNIFORM_VEC3);

    int lastLightId = 0;
    for (const auto& light : LightList)
    {
        if (light->Enabled)
        {
            int enabled = 1;
            SetShaderValue(LightShader.LightShader, LightShader.UniformLocations[lastLightId].EnabledLoc, &enabled, SHADER_UNIFORM_INT);

            light->UpdateShaderVars(LightShader, lastLightId);

            lastLightId++;
        }

        if (lastLightId >= MaxShaderLights)
            break;
    }

    while (lastLightId < MaxShaderLights)
    {
        int disabled = 0;
        SetShaderValue(LightShader.LightShader, LightShader.UniformLocations[lastLightId].EnabledLoc, &disabled, SHADER_UNIFORM_INT);
        lastLightId++;
    }
}

void Light::UpdateShaderVars(const ShaderInfo& lightShader, int lightId) const
{
    int type = int(LightType);

    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].ColorLoc, Intensity, SHADER_UNIFORM_VEC4);

    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].TypeLoc, &type, SHADER_UNIFORM_INT);

    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].PositionLoc, Position, SHADER_UNIFORM_VEC3);

    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].IntensityLoc, Intensity, SHADER_UNIFORM_VEC4);

    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].AttenuationLoc, &Attenuation, SHADER_UNIFORM_FLOAT);

    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].FalloffLoc, &Falloff, SHADER_UNIFORM_FLOAT);
}

void Light::SetPosition(const Vector3& pos)
{
    Position[0] = pos.x;
    Position[1] = pos.y;
    Position[2] = pos.z;

    SetDirty();
}

void Light::SetIntensity(const Color& color)
{
    Intensity[0] = color.r * ColorScale;
    Intensity[1] = color.g * ColorScale;
    Intensity[2] = color.b * ColorScale;

    SetDirty();
}

void Light::SetAttenuation(float attenuation)
{
    Attenuation = attenuation;
    SetDirty();
}

void Light::SetFalloff(float falloff)
{
    Falloff = falloff;
    SetDirty();
}

///--------DirectionalLight-------------

DirectionalLight::DirectionalLight()
{
    LightType = LightTypes::Directional;
}

void DirectionalLight::SetDirection(const Vector3& dir)
{
    Direction[0] = dir.x;
    Direction[1] = dir.y;
    Direction[2] = dir.z;
    SetDirty();
}

void DirectionalLight::UpdateShaderVars(const ShaderInfo& lightShader, int lightId) const
{
    Light::UpdateShaderVars(lightShader, lightId);
    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].DirectionLoc, Direction, SHADER_UNIFORM_VEC3);
}

///--------SpotLight-------------

SpotLight::SpotLight()
{
    LightType = LightTypes::Spot;
}

void SpotLight::SetConeAngle(const float &cone)
{
    Cone = cone;
    SetDirty();
}

void SpotLight::UpdateShaderVars(const ShaderInfo& lightShader, int lightId) const
{
    DirectionalLight::UpdateShaderVars(lightShader, lightId);
    SetShaderValue(lightShader.LightShader, lightShader.UniformLocations[lightId].ConeLoc, &Cone, SHADER_UNIFORM_FLOAT);
}
