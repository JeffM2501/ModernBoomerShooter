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

        DrawModelWires(model, Vector3Zeros, 1, SKYBLUE);

        BoundingBox bbox = GetModelBoundingBox(GetModel());
        DrawBoundingBox(bbox, PURPLE);

        EndMode3D();
    }
}