#pragma once

#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <vector>
#include <string_view>


class ModelInstance;

class ModelRecord
{
public:
    Model Geometry;

    BoundingBox GetBounds();

protected:
    size_t ReferenceCount = 0;
    BoundingBox Bounds = { 0 };
    bool BoundsValid = false;
protected:
    friend ModelInstance;
    std::shared_ptr<ModelInstance> GetModelInstance();

    void ReleaseInstance();

    void CheckBounds();
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