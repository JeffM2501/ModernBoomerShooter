#pragma once

#include "raylib.h"
#include "raymath.h"

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
    // a mesh and it's material index
    struct AnimateableMesh
    {
        Mesh Geometry;
    };

    // A bone with it's binding transform, and pointers to it's children
    struct AnimatableBoneInfo
    {
        std::string Name;
        size_t ParentBoneId = size_t(-1);
        Transform DefaultGlobalTransform;

        std::vector<AnimatableBoneInfo*> Children;
    };

    // A cache of the transforms to apply to a bone for a specific display pose
    struct AnimateablePose
    {
        std::vector<Matrix> BoneTransforms;
    };

    // A keyframe in an animation
    struct AnimateableKeyFrame
    {
        std::vector<Transform> GlobalTransforms;
    };

    // a sequence of animation keyframes
    struct AnimatableSequence
    {
        std::vector<AnimateableKeyFrame> Frames;
        float FPS = 30;
    };

    // a set of named animation sequences that can all be applied to the same skeleton
    struct AnimationSet
    {
        std::unordered_map<std::string, AnimatableSequence> Sequences;

        void Read(uint8_t* buffer, size_t size);
    };

    struct AnimatableMeshGroup
    {
        Material GroupMaterial;
        std::vector<AnimateableMesh> Meshes;
    };

    // a model that can be animated
    struct AnimateableModel
    {
        AnimateableModel() = default;
        // unloads on destruction
        virtual ~AnimateableModel();

        // noncopyable to prevent early unload
        AnimateableModel(const AnimateableModel&) = delete;
        AnimateableModel& operator = (const AnimateableModel&) = delete;

        
        std::vector<AnimatableMeshGroup> Groups;

        std::vector<AnimatableBoneInfo> Bones;

        // the root bone of the skeleton tree
        AnimatableBoneInfo* RootBone = nullptr;

        Matrix RootTransform = MatrixIdentity();

        void Read(uint8_t* buffer, size_t size);
        void Upload();
    };

    // loads an animated model from a raylib model, all the meshes and materials are transfered to the animateable model, and removed from the raylib model
    // do not call UnloadModel on the model
    void LoadFromModel(AnimateableModel& animModel, const Model& model);

    // loads an animation set from a raylib animation array, all the sequences are transfered to the animation set and each animation is unloaded
    // you are responsible for deleting the array memory, but not the animations it points to
    void LoadFromAnimation(AnimationSet & animSet, const AnimateableModel& model, ModelAnimation* animationsPointer, size_t count);

    // creates a pose in the default (bind pose) position that can be animated.
    AnimateablePose GetDefaultPose(const AnimateableModel& model);

    // updates a pose to match a keyframe
    void UpdatePoseToFrame(const AnimateableModel& model, AnimateablePose& pose, const AnimateableKeyFrame& frame);
    
    // updates a pose to be an interpolation value between two keyframes
    void InterpolatePose(const AnimateableModel& model, AnimateablePose& pose, const AnimateableKeyFrame& frame1, const AnimateableKeyFrame& frame2, float param);

    // draws a model, with transform, at a pose, with a set of optional material overrides
    void DrawAnimatableModel(const AnimateableModel& model, Matrix transform, AnimateablePose* pose = nullptr, const std::vector<Material>* materialOverrides = nullptr);
}