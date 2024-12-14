#include "transform_tools.h"

#include "app.h"

#include "imgui.h"
#include "extras/IconsFontAwesome6.h"

#include "raylib.h"
#include "raymath.h"

namespace TransformTools
{
    void CenterMesh()
    {
        BoundingBox bbox = GetModelBoundingBox(App::GetModel());
        Vector3 center = (bbox.max - bbox.min) / 2 + bbox.min;

        for (size_t meshIndex = 0; meshIndex < App::GetModel().meshCount; meshIndex++)
        {
            Mesh& mesh = App::GetModel().meshes[meshIndex];
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
        BoundingBox bbox = GetModelBoundingBox(App::GetModel());

        for (size_t meshIndex = 0; meshIndex < App::GetModel().meshCount; meshIndex++)
        {
            Mesh& mesh = App::GetModel().meshes[meshIndex];
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

        for (size_t meshIndex = 0; meshIndex < App::GetModel().meshCount; meshIndex++)
        {
            Mesh& mesh = App::GetModel().meshes[meshIndex];
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
}



TransformPanel::TransformPanel()
{
    Name = "Transformations";
    Icon = ICON_FA_UP_DOWN_LEFT_RIGHT;
    MinSize = { 250,100 };

    ExtraWindowFlags = ImGuiWindowFlags_NoResize;
}

void TransformPanel::OnShowContents()
{
    if (ImGui::Button("Center"))
        TransformTools::CenterMesh();

    if (ImGui::Button("Floor"))
        TransformTools::FloorMesh();

    if (ImGui::Button("Y -> Z"))
        TransformTools::RotateMesh(90, Vector3UnitX);
}