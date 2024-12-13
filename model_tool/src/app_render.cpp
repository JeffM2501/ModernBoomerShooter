#include "app.h"

#include "imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

namespace App
{
    Camera3D ViewCamera = { 0 };

    void InitRender()
    {
        ViewCamera.fovy = 45;
        ViewCamera.up.z = 1;

        ViewCamera.position.z = 2;
        ViewCamera.position.y = -2;
    }

    void CleanupRender()
    {

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
        BeginMode3D(ViewCamera);

        rlPushMatrix();
        rlRotatef(90, 1, 0, 0);
        DrawGrid(20, 1);
        rlPopMatrix();
        DrawLine3D(Vector3Zeros, Vector3{ 0, 0, 10 }, DARKGREEN);

        DrawModel(GetModel(), Vector3Zeros, 1, WHITE);

        BoundingBox bbox = GetModelBoundingBox(GetModel());
        DrawBoundingBox(bbox, PURPLE);

        EndMode3D();
    }
}