#pragma once

#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <vector>
#include <string_view>

class ModelRecord
{
public:
    Model Geometry;

protected:
    size_t ReferenceCount = 0;

protected:
    friend class ModelInstance;
    std::shared_ptr<ModelInstance> GetModelInstance();

    void ReleaseInstance();
};

class ModelInstance
{
public:
    ~ModelInstance();
public:
    ModelRecord* Geometry;
    void Draw(class TransformComponent& transform);

    ModelInstance(ModelRecord* geomeetry);

    void SetShader(Shader shader);

private:
    Shader ModelShader = { 0 };

    std::vector<Material> MaterialOverrides;
};

namespace ModelManager
{
    void Init();
    void Cleanup();

    std::shared_ptr<ModelInstance> GetModel(std::string_view name);
    void UnloadAll();
};