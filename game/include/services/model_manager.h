#pragma once

#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>

class ModelInstance;

class ModelRecord
{
public:
    virtual ~ModelRecord() = default;

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

class AnimatedModelRecord : public ModelRecord
{
public:
    ModelAnimation* AnimationsPointer = nullptr;
    size_t AnimationsCount = 0;

    std::unordered_map<std::string, ModelAnimation*> AnimationSequences;
};

class ModelInstance
{
public:
    virtual ~ModelInstance();
public:
    ModelRecord* Geometry;
    virtual void Draw(class TransformComponent& transform);

    ModelInstance(ModelRecord* geomeetry);

    void SetShader(Shader shader);

protected:
    Shader ModelShader = { 0 };

    std::vector<Material> MaterialOverrides;
};

class AnimatedModelInstance : public ModelInstance
{
public:
    AnimatedModelRecord* AnimatedModel = nullptr;

    AnimatedModelInstance(AnimatedModelRecord* model);

    void Advance(float dt);
    void Draw(class TransformComponent& transform) override;
    
    void SetSequence(const std::string& name, int startFrame = 0);

    void SetAnimationFPS(int fps) { AnimationFPS = fps; }

protected:
    std::vector<Matrix> PoseMatricies;

    int AnimationFPS = 30;

    ModelAnimation* CurrentSequence = nullptr;
    int CurrentFrame = 0;
    float CurrentParam = 0;
};

namespace ModelManager
{
    void Init();
    void Cleanup();

    std::shared_ptr<ModelInstance> GetModel(std::string_view name);
    std::shared_ptr<AnimatedModelInstance> GetAnimatedModel(std::string_view name);
    void UnloadAll();
};