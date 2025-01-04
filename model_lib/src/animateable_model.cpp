#include "model.h"

#include "raymath.h"
#include "rlgl.h"
#include <set>

namespace Models
{
    AnimateableModel::~AnimateableModel()
    {
        for (auto& group : Groups)
        {
            MemFree(group.GroupMaterial.maps);

            for (auto& mesh : group.Meshes)
                UnloadMesh(mesh.Geometry);
        }
    }

    void AnimateableModel::Upload()
    {
        for (auto& group : Groups)
        {
            for (auto& mesh : group.Meshes)
                UploadMesh(&mesh.Geometry, false); // we only do GPU animation here
        }
    }

    BoundingBox AnimateableModel::GetBounds()
    {
        BoundingBox bbox = { 0 };
        bool valid = false;
        if (!Groups.empty())
        {
            Vector3 temp = { 0 };
            for (auto& group : Groups)
            {
                for (auto& mesh : group.Meshes)
                {
                    if (!valid)
                    {
                        bbox = GetMeshBoundingBox(mesh.Geometry);
                        valid = true;
                        break;
                    }
                    BoundingBox tempBounds = GetMeshBoundingBox(mesh.Geometry);

                    temp.x = (bbox.min.x < tempBounds.min.x) ? bbox.min.x : tempBounds.min.x;
                    temp.y = (bbox.min.y < tempBounds.min.y) ? bbox.min.y : tempBounds.min.y;
                    temp.z = (bbox.min.z < tempBounds.min.z) ? bbox.min.z : tempBounds.min.z;
                    bbox.min = temp;

                    temp.x = (bbox.max.x > tempBounds.max.x) ? bbox.max.x : tempBounds.max.x;
                    temp.y = (bbox.max.y > tempBounds.max.y) ? bbox.max.y : tempBounds.max.y;
                    temp.z = (bbox.max.z > tempBounds.max.z) ? bbox.max.z : tempBounds.max.z;
                    bbox.max = temp;
                }
            }
        }

        return bbox;
    }

    void LoadFromModel(AnimateableModel& animModel, const Model& model)
    {
        animModel.Groups.clear();

        animModel.Groups.resize(model.materialCount);

        // copy the meshes
        for (int i = 0; i < model.meshCount; i++)
        {
            auto& group = animModel.Groups[model.meshMaterial[i]];

            group.Meshes.emplace_back();
            group.Meshes.back().Geometry = model.meshes[i];

            // forcibly clear the bone matricides since we are going to handle them externally
            MemFree(group.Meshes.back().Geometry.boneMatrices);
            group.Meshes.back().Geometry.boneMatrices = nullptr;
        }

        MemFree(model.meshes);

        // copy the materials
        for (int i = 0; i < model.materialCount; i++)
        {
            animModel.Groups[i].GroupMaterial = model.materials[i];
        }

        MemFree(model.materials);

        // make the bone list
        for (int i = 0; i < model.boneCount; i++)
        {
            animModel.Bones.emplace_back();
            animModel.Bones.back().Name = model.bones[i].name;
            animModel.Bones.back().ParentBoneId = model.bones[i].parent;
            animModel.Bones.back().DefaultGlobalTransform = model.bindPose[i];
        }

        // build the bone tree
        for (auto& bone : animModel.Bones)
        {
            if (bone.ParentBoneId == -1)
                animModel.RootBone = &bone;
            else
                animModel.Bones[bone.ParentBoneId].Children.push_back(&bone);
        }

        if (model.bones)
            MemFree(model.bones);

        if (model.bindPose)
            MemFree(model.bindPose);
    }

    void LoadFromAnimation(AnimationSet& animSet, const AnimateableModel& model, ModelAnimation* animationsPointer, size_t count)
    {
        animSet.Sequences.clear();

        for (size_t i = 0; i < count; i++)
        {
            auto& sequence = animSet.Sequences.try_emplace(std::string(animationsPointer[i].name)).first->second;

            for (int f = 0; f < animationsPointer[i].frameCount; f++)
            {
                sequence.Frames.emplace_back();
                for (size_t b = 0; b < model.Bones.size(); b++)
                {
                    sequence.Frames.back().GlobalTransforms.push_back(animationsPointer[i].framePoses[f][b]);
                }

                MemFree(animationsPointer[i].framePoses[f]);
            }

            MemFree(animationsPointer[i].framePoses);
            MemFree(animationsPointer[i].bones);
        }
    }

    AnimateablePose GetDefaultPose(const AnimateableModel& model)
    {
        AnimateablePose pose;
        pose.BoneTransforms.resize(model.Bones.size());
        for (auto& bone : pose.BoneTransforms)
            bone = MatrixIdentity();

        return pose;
    }

    Matrix GetBoneMatrix(const Transform& bindingTransform, const Transform& frameTransform)
    {
        Vector3 inTranslation = bindingTransform.translation;
        Quaternion inRotation = bindingTransform.rotation;
        Vector3 inScale = bindingTransform.scale;

        Vector3 outTranslation = frameTransform.translation;
        Quaternion outRotation = frameTransform.rotation;
        Vector3 outScale = frameTransform.scale;

        Vector3 invTranslation = Vector3RotateByQuaternion(Vector3Negate(inTranslation), QuaternionInvert(inRotation));
        Quaternion invRotation = QuaternionInvert(inRotation);
        Vector3 invScale = Vector3Divide(Vector3Ones, inScale);

        Vector3 boneTranslation = Vector3Add(
            Vector3RotateByQuaternion(Vector3Multiply(outScale, invTranslation),
                outRotation), outTranslation);
        Quaternion boneRotation = QuaternionMultiply(outRotation, invRotation);
        Vector3 boneScale = Vector3Multiply(outScale, invScale);

        Matrix boneMatrix = MatrixMultiply(MatrixMultiply(
            QuaternionToMatrix(boneRotation),
            MatrixTranslate(boneTranslation.x, boneTranslation.y, boneTranslation.z)),
            MatrixScale(boneScale.x, boneScale.y, boneScale.z));

        return boneMatrix;
    }

    void UpdatePoseToFrame(const AnimateableModel& model, AnimateablePose& pose, const AnimateableKeyFrame& frame)
    {
        for (size_t boneId = 0; boneId < model.Bones.size(); boneId++)
        {
            const auto& bone = model.Bones[boneId];
            pose.BoneTransforms[boneId] = GetBoneMatrix(bone.DefaultGlobalTransform, frame.GlobalTransforms[boneId]);
        }
    }

    Transform TransformLerp(const Transform& t1, const Transform& t2, float param)
    {
        return Transform {  Vector3Lerp(t1.translation, t2.translation, param),
                            QuaternionSlerp(t1.rotation, t2.rotation, param),
                            Vector3Lerp(t1.scale, t2.scale, param) };
    }

    void InterpolatePose(const AnimateableModel& model, AnimateablePose& pose, const AnimateableKeyFrame& frame1, const AnimateableKeyFrame& frame2, float param)
    {
        for (size_t boneId = 0; boneId < model.Bones.size(); boneId++)
        {
            const auto& bone = model.Bones[boneId];
            pose.BoneTransforms[boneId] = GetBoneMatrix(bone.DefaultGlobalTransform, TransformLerp(frame1.GlobalTransforms[boneId], frame2.GlobalTransforms[boneId], param));
        }
    }

    void DrawAnimatableModel(const AnimateableModel& model, Matrix transform, AnimateablePose* pose, const std::vector<Material>* materialOverrides)
    {
        int groupId = 0;
        for (auto& group : model.Groups)
        {
            if (group.Meshes.empty())
                continue;

            const Material* materialToUse = &group.GroupMaterial;
            if (materialOverrides)
                materialToUse = &(*materialOverrides)[groupId];

            if (pose && materialToUse->shader.locs[SHADER_LOC_BONE_MATRICES] != -1)
            {
                rlEnableShader(materialToUse->shader.id);
                rlSetUniformMatrices(materialToUse->shader.locs[SHADER_LOC_BONE_MATRICES], &pose->BoneTransforms[0], int(model.Bones.size()));
            }

            for (auto& mesh : group.Meshes)
            {
                DrawMesh(mesh.Geometry, *materialToUse, transform);
            }
            groupId++;
        }
    }
}