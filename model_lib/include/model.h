#pragma once

#include "raylib.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <stdint.h>
#include <functional>

void WriteModel(Model& model, std::string_view file);
void ReadModel(Model& model, uint8_t* buffer, size_t size, bool supportCPUAnimation = false);

void WriteModelAnimations(ModelAnimation* animations, size_t count, std::string_view file);
ModelAnimation* ReadModelAnimations(const Model& model, size_t& count, uint8_t* buffer, size_t size);

using ResolveModelTextureCallback = std::function<Texture(std::string_view)>;
void SetModelTextureResolver(ResolveModelTextureCallback callback);

namespace Models
{
    struct AnimateableMesh
    {
        Mesh Geometry;
        size_t MaterialIndex = 0;
    };

    struct AnimatableBoneInfo
    {
        std::string Name;
        size_t ParentBoneId = size_t(-1);
        Transform DefaultGlobalTransform;

        std::vector<AnimatableBoneInfo*> Children;
    };

    struct AnimateablePose
    {
        std::vector<Matrix> BoneTransforms;
    };

    struct AnimateableKeyFrame
    {
        std::vector<Transform> GlobalTransforms;
    };

    struct AnimatableSequence
    {
        std::vector<AnimateableKeyFrame> Frames;
        std::unordered_map<size_t, std::string> EventFrames;
        float FPS = 30;
    };

    struct AnimationSet
    {
        std::unordered_map<std::string, AnimatableSequence> Sequences;
    };

    struct AnimateableModel
    {
        virtual ~AnimateableModel();
        AnimateableModel() = default;
        AnimateableModel(const AnimateableModel&) = delete;
        AnimateableModel& operator = (const AnimateableModel&) = delete;

        std::vector<AnimateableMesh> Meshes;
        std::vector<Material> Materials;

        std::vector<AnimatableBoneInfo> Bones;

        AnimatableBoneInfo* RootBone = nullptr;
    };

    // model is unloaded after this call
    void LoadFromModel(AnimateableModel& animModel, const Model& model);

    void LoadFromAnimation(AnimationSet & animSet, const AnimateableModel& model, ModelAnimation* animationsPointer, size_t count);

    AnimateablePose GetDefaultPose(const AnimateableModel& model);
    void UpdatePoseToFrame(const AnimateableModel& model, AnimateablePose& pose, const AnimateableKeyFrame& frame);
    void InterpolatePose(const AnimateableModel& model, AnimateablePose& pose, const AnimateableKeyFrame& frame1, const AnimateableKeyFrame& frame2, float param);

    void DrawAnimatableModel(const AnimateableModel& model, Matrix transform, AnimateablePose* pose = nullptr, const std::vector<Material>* materialOverrides = nullptr);
}