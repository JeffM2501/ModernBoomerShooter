#include "app.h"

#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

namespace App
{
    Camera3D ViewCamera = { 0 };

    Shader LightingShader = { 0 };

    static constexpr char ViewPosName[] = "viewPos";
    static constexpr char AmbientName[] = "ambient";

    static float Ambient[4] = { 0.15f ,0.15f, 0.15f, 1.0f };

    int AmbientLoc = 0;

    int SelectedMeshIndex = -1;
    int SelectedBoneIndex = -1;

    bool ShowBones = true;

    BoundingBox SelectedMeshBBox = { 0 };

    void SetSeletedMesh(int mesh)
    {
        SelectedMeshIndex = mesh;

//         auto& model = GetModel();
//         if (IsModelValid(model) && mesh >= 0 && mesh < model.meshCount)
//         {
//             SelectedMeshBBox = GetMeshBoundingBox(model.meshes[mesh]);
//         }
//         else
//         {
//             SelectedMeshIndex = -1;
//         }
    }

    int GetSelectedMesh()
    {
        return SelectedMeshIndex;
    }

    void SetSeletedBone(int bone)
    {
        SelectedBoneIndex = bone;
    }

    int GetSelectedBone()
    {
        return SelectedBoneIndex;
    }

    void ModelUpdated()
    {
        for (auto& g : App::GetModel().Groups)
            g.GroupMaterial.shader = LightingShader;
    }

    void InitRender()
    {
        ViewCamera.fovy = 45;
        ViewCamera.up.y = 1;

        ViewCamera.position.y = 2;
        ViewCamera.position.z = -2;

        LightingShader = LoadShader("resources/shaders/world.vs", "resources/shaders/world.fs");

        LightingShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(LightingShader, ViewPosName);

        AmbientLoc = GetShaderLocation(LightingShader, AmbientName);

        float ambientScale = 0.125f;

        float tintScale = 1.5f;
        SetShaderValue(LightingShader, GetShaderLocation(LightingShader, "tintScale"), &tintScale, SHADER_UNIFORM_FLOAT);

        float globalColor[4] = { 1, 1, 1, 1 };
        SetShaderValue(LightingShader, GetShaderLocation(LightingShader, "gloablLightColor"), globalColor, SHADER_UNIFORM_VEC4);

        float backFillColor[4] = { 0.75f, 0.75f, 0.75f, 1 };
        SetShaderValue(LightingShader, GetShaderLocation(LightingShader, "gloablBackfillLightColor"), backFillColor, SHADER_UNIFORM_VEC4);

        float ambientAngle = -45;

        Vector3 lightVec = {
            sinf(DEG2RAD * ambientAngle),
            cosf(DEG2RAD * ambientAngle),
            -1.0f
        };
        lightVec = Vector3Normalize(lightVec);

        SetShaderValue(LightingShader, GetShaderLocation(LightingShader, "gloablLightDirection"), &lightVec, SHADER_UNIFORM_VEC3);

        lightVec = Vector3Scale(lightVec, -1);
        SetShaderValue(LightingShader, GetShaderLocation(LightingShader, "gloablBackfillLightDirection"), &lightVec, SHADER_UNIFORM_VEC3);
    }

    void CleanupRender()
    {
        UnloadShader(LightingShader);
        LightingShader.id = 0;
    }

    void UpdateRender()
    {
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
                UpdateCamera(&ViewCamera, CAMERA_THIRD_PERSON);
        }
    }

    void DrawPose(Models::AnimateableModel& model, Models::AnimateableKeyFrame *pose)
    {
        for (size_t i = 0; i < model.Bones.size(); i++)
        {
            auto& bone = model.Bones[i];
            Transform* transform = &bone.DefaultGlobalTransform;
            if (pose)
            {
                transform = &pose->GlobalTransforms[i];
            }

            Vector3 pos = transform->translation;

            DrawSphere(pos, 0.01f, i == SelectedBoneIndex ? YELLOW : MAROON);

            Vector3 vec = Vector3RotateByQuaternion(Vector3{ 0, 0.125f, 0 }, transform->rotation);
            DrawLine3D(pos, Vector3Add(vec, pos), BLUE);

            if (bone.ParentBoneId < model.Bones.size())
            {
                Transform* parentTransform = &model.Bones[bone.ParentBoneId].DefaultGlobalTransform;
                if (pose)
                {
                    parentTransform = &pose->GlobalTransforms[bone.ParentBoneId];
                }

                Vector3 parentPos = parentTransform->translation;

                DrawLine3D(pos, parentPos, i == SelectedBoneIndex ? ORANGE : PURPLE);
            }
        }
    }

    void DrawRender()
    {
        auto& model = GetModel();

        int animationShaderLocation = GetShaderLocation(LightingShader, "animate");
        int val = 1;
        SetShaderValue(LightingShader, animationShaderLocation, &val, SHADER_UNIFORM_INT);

        SetShaderValue(LightingShader, AmbientLoc, Ambient, SHADER_UNIFORM_VEC4);
        SetShaderValue(LightingShader, LightingShader.locs[SHADER_LOC_VECTOR_VIEW], &ViewCamera.position, SHADER_UNIFORM_VEC3);

        BeginMode3D(ViewCamera);

        rlPushMatrix();
        //rlRotatef(90, 1, 0, 0);
        DrawGrid(20, 1);
        rlPopMatrix();
        DrawLine3D(Vector3Zeros, Vector3{ 0, 10, 0 }, DARKBLUE);
        DrawLine3D(Vector3Zeros, Vector3{ 1, 0, 0 }, MAROON);
        DrawLine3D(Vector3Zeros, Vector3{ 0, 0, 1 }, DARKGREEN);

        auto& anims = GetAnimations();
        Models::AnimateableKeyFrame* keyFrame = nullptr;
        if (!anims.Animations.Sequences.empty() && anims.Frame >= 0 && !anims.Sequence.empty())
            keyFrame = &anims.Animations.Sequences[anims.Sequence].Frames[anims.Frame];

        auto pose = Models::GetDefaultPose(model);

        if (keyFrame)
            Models::UpdatePoseToFrame(model, pose, *keyFrame);

        Models::DrawAnimatableModel(model, MatrixIdentity(), &pose, nullptr);

     //   DrawModel(GetModel(), Vector3Zeros, 1, WHITE);

        if (SelectedMeshIndex >= 0)
        {
            DrawBoundingBox(SelectedMeshBBox, BLUE);
        }

        if (!model.Bones.empty() && ShowBones)
        {
            rlDrawRenderBatchActive();
            rlDisableDepthTest();

            DrawPose(model, keyFrame);

            rlDrawRenderBatchActive();
            rlEnableDepthTest();
        }

        EndMode3D();
    }
}