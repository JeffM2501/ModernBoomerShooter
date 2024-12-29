#pragma once

#include "model.h"
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

    Models::AnimateableModel ModelGeometry;

    BoundingBox GetBounds();

    Matrix OrientationTransform = MatrixIdentity();

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
    Models::AnimationSet Animations;
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

    void SetAnimationFPSMultiplyer(float value) { AnimationFPSMultiply = value; }

protected:
    int AnimationFPS = 90;

    int CurrentFrame = 0;
    float CurrentParam = 0;

    Models::AnimatableSequence* CurrentAnimaton = nullptr;

    Models::AnimateablePose CurrentPose;

    float AnimationAccumulator = 0;

    float AnimationFPSMultiply = 1;
};

namespace ModelManager
{
    void Init();
    void Cleanup();

    std::shared_ptr<ModelInstance> GetModel(std::string_view name);
    std::shared_ptr<AnimatedModelInstance> GetAnimatedModel(std::string_view name);
    void UnloadAll();
};