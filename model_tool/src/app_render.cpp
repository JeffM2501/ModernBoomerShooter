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

        auto& model = GetModel();
        if (IsModelValid(model) && mesh >= 0 && mesh < model.meshCount)
        {
            SelectedMeshBBox = GetMeshBoundingBox(model.meshes[mesh]);
        }
        else
        {
            SelectedMeshIndex = -1;
        }
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

    void RebuildAnimFrame()
    {
        auto& anim = GetAnimations();
        if (anim.Animations.empty() || anim.Frame < 0 || anim.Sequence < 0)
            return;

        UpdateModelAnimation(GetModel(), *anim.Animations[anim.Sequence], anim.Frame);
    }

    void InitRender()
    {
        ViewCamera.fovy = 45;
        ViewCamera.up.z = 1;

        ViewCamera.position.z = 2;
        ViewCamera.position.y = -2;

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

        float ambientAngle =  -45;

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

    void DrawPose(Transform* pose)
    {
        auto& model = GetModel();

        for (int boneId = 0; boneId < model.boneCount; boneId++)
        {
            Transform& bone = pose[boneId];
            int boneParent = model.bones[boneId].parent;

            Vector3 pos = bone.translation;

            DrawSphere(pos, 0.01f, boneId == SelectedBoneIndex ? YELLOW : MAROON);

            Vector3 vec = Vector3RotateByQuaternion(Vector3{ 0, 0.125f, 0 }, bone.rotation);
            DrawLine3D(pos, Vector3Add(vec, pos), BLUE);

            if (boneParent >= 0)
            {
                Transform& parentBone = pose[boneParent];
                Vector3 parentPos = parentBone.translation;

                DrawLine3D(pos, parentPos, boneId == SelectedBoneIndex ? ORANGE : PURPLE);
            }
        }
    }

    void DrawRender()
    {
        auto& model = GetModel();

        for (int i = 0; i < model.materialCount; i++)
        {
            model.materials[i].shader = LightingShader;
        }

        SetShaderValue(LightingShader, AmbientLoc, Ambient, SHADER_UNIFORM_VEC4);
        SetShaderValue(LightingShader, LightingShader.locs[SHADER_LOC_VECTOR_VIEW], &ViewCamera.position, SHADER_UNIFORM_VEC3);

        BeginMode3D(ViewCamera);

        rlPushMatrix();
        rlRotatef(90, 1, 0, 0);
        DrawGrid(20, 1);
        rlPopMatrix();
        DrawLine3D(Vector3Zeros, Vector3{ 0, 0, 10 }, DARKGREEN);

        DrawModel(GetModel(), Vector3Zeros, 1, WHITE);

        if (SelectedMeshIndex >= 0)
        {
            DrawBoundingBox(SelectedMeshBBox, BLUE);
        }

        if (model.boneCount > 0 && ShowBones)
        {
            rlDrawRenderBatchActive();
            rlDisableDepthTest();

            auto& anims = GetAnimations();
            if (anims.Animations.empty() || anims.Frame < 0 || anims.Sequence < 0)
                DrawPose(model.bindPose);
            else
                DrawPose(anims.Animations[anims.Sequence]->framePoses[anims.Frame]);
            
            rlDrawRenderBatchActive();
            rlEnableDepthTest();
        }

        EndMode3D();
    }
}