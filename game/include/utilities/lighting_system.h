#pragma once

#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <memory>
#include "raylib.h"

enum class LightTypes
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

struct ShaderInfo
{
    Shader LightShader;

    struct ShaderUniformLocs
    {
        int EnabledLoc = -1;
        int TypeLoc = -1;
        int PositionLoc = -1;
        int ColorLoc = -1;
        int IntensityLoc = -1;
        int AttenuationLoc = -1;
        int FalloffLoc = -1;
        int ConeLoc = -1;
        int DirectionLoc = -1;
    };
    std::vector<ShaderUniformLocs> UniformLocations;

    int AmbientLoc = -1;
};

class Light
{
public:
    virtual ~Light() = default;

    bool IsDirty() const { return Dirty; }

    LightTypes GetType() const { return LightType; }

    void SetPosition(const Vector3& pos);
    void SetIntensity(const Color& color);
    void SetAttenuation(float attenuation);
    void SetFalloff(float falloff);

    virtual void UpdateShaderVars(const ShaderInfo& shader, int lightId) const;

    bool Enabled = true;

protected:
    LightTypes LightType = LightTypes::Directional;

    float Position[3] = { 0,0,0 };
    float Intensity[4] = { 1,1,1,1 };

    float Attenuation = 5;
    float Falloff = 10;

    void SetDirty() { Dirty = true; }

private:
    bool Dirty = false;
    std::string FieldNameCache;
};

class PointLight : public Light
{
public:
    PointLight() { LightType = LightTypes::Point; }
};

class DirectionalLight : public Light
{
public:
    DirectionalLight();

    void SetDirection(const Vector3& dir);

    void UpdateShaderVars(const ShaderInfo& shader, int lightId) const override;

protected:
    float Direction[3] = { 1,1,-1 };
};

class SpotLight : public DirectionalLight
{
public:
    SpotLight();

    void SetConeAngle(const float& cone);
    void UpdateShaderVars(const ShaderInfo& shader, int lightId) const override;

protected:
    float Cone = 45;
};

class LightScene
{
public:
    static constexpr int MaxShaderLights = 4; // Max dynamic lights supported by shader
    
    void SetShader(Shader shader);
    Shader GetShader();

    void SetAmbientColor(Color color);
    void SetAmbientColor(float luminance);

    Light* AddLight(LightTypes lightType);
    void RemoveLight(Light* light);

    void ClearLights();

    void ApplyLights(const Camera3D& viewportCamear);

private:
    ShaderInfo LightShader;

    float Ambient[4] = { 0.05f ,0.05f, 0.05f, 1.0f };
    std::list<std::unique_ptr<Light>> LightList;
};