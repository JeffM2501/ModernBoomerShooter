#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "model.h"

namespace App
{
    bool Run = false;


    Model TheModel = { 0 };

    Camera3D ViewCamera = { 0 };

    const char* ModelName = nullptr;

    void Init()
    {
        Run = true;
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(1280, 800, "Model Tool");
        SetTargetFPS(300);

        ViewCamera.fovy = 45;
        ViewCamera.up.z = 1;

        ViewCamera.position.z = 2;
        ViewCamera.position.y = -2;
    }

    void CenterMesh()
    {
        BoundingBox bbox = GetModelBoundingBox(TheModel);
        Vector3 center = (bbox.max - bbox.min)/2 + bbox.min;

        for (size_t meshIndex = 0; meshIndex < TheModel.meshCount; meshIndex++)
        {
            Mesh& mesh = TheModel.meshes[meshIndex];
            for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
            {
                mesh.vertices[vertIndex * 3 + 0] -= center.x;
                mesh.vertices[vertIndex * 3 + 1] -= center.y;
                mesh.vertices[vertIndex * 3 + 2] -= center.z;
            }

            UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
        }
    }

    void FloorMesh()
    {
        BoundingBox bbox = GetModelBoundingBox(TheModel);

        for (size_t meshIndex = 0; meshIndex < TheModel.meshCount; meshIndex++)
        {
            Mesh& mesh = TheModel.meshes[meshIndex];
            for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
            {
                mesh.vertices[vertIndex * 3 + 2] -= bbox.min.z;
            }

            UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
        }
    }

    void RotateMesh(float angle, Vector3 axis)
    {
        Matrix mat = MatrixRotate(axis, angle * DEG2RAD);

        for (size_t meshIndex = 0; meshIndex < TheModel.meshCount; meshIndex++)
        {
            Mesh& mesh = TheModel.meshes[meshIndex];
            for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
            {
                Vector3 vert = { mesh.vertices[vertIndex * 3 + 0] , mesh.vertices[vertIndex * 3 + 1] , mesh.vertices[vertIndex * 3 + 2] };
                Vector3 norm = { mesh.normals[vertIndex * 3 + 0] , mesh.normals[vertIndex * 3 + 1] , mesh.normals[vertIndex * 3 + 2] };

                vert = vert * mat;
                norm = norm * mat;

                mesh.vertices[vertIndex * 3 + 0] = vert.x;
                mesh.vertices[vertIndex * 3 + 1] = vert.y;
                mesh.vertices[vertIndex * 3 + 2] = vert.z;

                mesh.normals[vertIndex * 3 + 0] = norm.x;
                mesh.normals[vertIndex * 3 + 1] = norm.y;
                mesh.normals[vertIndex * 3 + 2] = norm.z;
            }

            UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
            UpdateMeshBuffer(mesh, 2, mesh.normals, mesh.vertexCount * 3 * sizeof(float), 0);
        }
    }

    void NewFrame()
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            UpdateCamera(&ViewCamera, CAMERA_THIRD_PERSON);

        if (IsKeyPressed(KEY_R))
            RotateMesh(90, Vector3UnitX);
        
        if (IsKeyPressed(KEY_F))
            FloorMesh();

        if (IsKeyPressed(KEY_O))
            WriteModel(TheModel, ModelName);

        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode3D(ViewCamera);

        rlPushMatrix();
        rlRotatef(90, 1, 0, 0);
        DrawGrid(20, 1);
        rlPopMatrix();
        DrawLine3D(Vector3Zeros, Vector3{ 0, 0, 10 }, DARKGREEN);

        DrawModel(TheModel, Vector3Zeros, 1, WHITE);

        BoundingBox bbox = GetModelBoundingBox(TheModel);
        DrawBoundingBox(bbox, PURPLE);

        EndMode3D();

        EndDrawing();
    }

    void Cleanup()
    {
        CloseWindow();
    }

    bool Quit()
    {
        return !Run || WindowShouldClose();
    }
}

void ProcessModel(const char* name)
{
    if (IsModelValid(App::TheModel))
        UnloadModel(App::TheModel);

    App::ModelName = GetFileNameWithoutExt(name);

    App::TheModel = LoadModel(name);

    if (IsModelValid(App::TheModel))
    {
        App::CenterMesh();
        App::RotateMesh(90, Vector3UnitX);
        App::FloorMesh();
        WriteModel(App::TheModel, App::ModelName);
    }
}

int main(int argc, char* argv[])
{
    App::Init();

    if (argc == 1)
    { 
        auto files = LoadDirectoryFilesEx("assets/models", ".glb", false);
        for (size_t i = 0; i < files.count; i++)
        {
            ProcessModel(files.paths[i]);
        }
    }
    else
    {
        ProcessModel(argv[1]);
    }
    
    while (!App::Quit())
    {
        App::NewFrame();
    }
    App::Cleanup();

    return 0;
}