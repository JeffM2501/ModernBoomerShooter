#include "transform_tools.h"

#include "app.h"

#include "imgui.h"
#include "extras/IconsFontAwesome6.h"

#include "config.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <string_view>

#include <vector>
#include <map>

namespace TransformTools
{
    void CenterMesh()
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        BoundingBox bbox = GetModelBoundingBox(model);

        Vector3 center = (bbox.max - bbox.min) / 2 + bbox.min;

        for (size_t meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
        {
            Mesh& mesh = model.meshes[meshIndex];
            for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
            {
                mesh.vertices[vertIndex * 3 + 0] -= center.x;
                mesh.vertices[vertIndex * 3 + 1] -= center.y;
                mesh.vertices[vertIndex * 3 + 2] -= center.z;
            }

            UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
        }

        for (int i = 0; i < model.boneCount; i++)
        {
            model.bindPose[i].translation -= center;
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    void FloorMesh()
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        BoundingBox bbox = GetModelBoundingBox(model);

        for (size_t meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
        {
            Mesh& mesh = model.meshes[meshIndex];
            for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
            {
                mesh.vertices[vertIndex * 3 + 2] -= bbox.min.z;
            }

            UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
        }

        for (int i = 0; i < model.boneCount; i++)
        {
            model.bindPose[i].translation.z -= bbox.min.z;
        }

        for (auto anim : App::GetAnimations().Animations)
        {
            for (int f = 0; f < anim->frameCount; f++)
            {
                for (int i = 0; i < model.boneCount; i++)
                {
                    anim->framePoses[f][i].translation.z -= bbox.min.z;
                }
            }
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    void RotateMesh(float angle, Vector3 axis)
    {
        Matrix mat = MatrixRotate(axis, angle * DEG2RAD);
        Quaternion rot = QuaternionFromMatrix(mat);

        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        for (size_t meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
        {
            Mesh& mesh = model.meshes[meshIndex];
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

        for (int i = 0; i < model.boneCount; i++)
        {
            model.bindPose[i].translation *= mat;
            model.bindPose[i].rotation = QuaternionMultiply(model.bindPose[i].rotation, rot);
        }

        for (auto anim : App::GetAnimations().Animations)
        {
            for (int f = 0; f < anim->frameCount; f++)
            {
                for (int i = 0; i < model.boneCount; i++)
                {
                    anim->framePoses[f][i].translation *= mat;
                    anim->framePoses[f][i].rotation = QuaternionMultiply(anim->framePoses[f][i].rotation, rot);
                }
            }
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    static std::hash<std::string_view> Hasher;

    bool IsSameMaterialMap(MaterialMap& map1, MaterialMap& map2)
    {
        if (ColorToInt(map1.color) != ColorToInt(map2.color))
            return false;

        if (map1.texture.id == map2.texture.id)
            return true;

        Image img1 = LoadImageFromTexture(map1.texture);
        Image img2 = LoadImageFromTexture(map1.texture);

        std::string_view view1((const char*)img1.data, GetPixelDataSize(img1.width,img1.height,img1.format));
        size_t hash1= Hasher(view1);

        std::string_view view2((const char*)img1.data, GetPixelDataSize(img2.width, img2.height, img2.format));
        size_t hash2 = Hasher(view2);

        UnloadImage(img1);
        UnloadImage(img2);

        return hash1 == hash2;
    }

    bool IsSameMaterial(Material& mat1, Material& mat2)
    {
        if (mat1.shader.id != mat2.shader.id)
            return false;

        for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
        {
            if (!IsSameMaterialMap(mat1.maps[i], mat2.maps[i]))
                return false;
        }

        return true;
    }

    void BakeMaterialColors()
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        for (int i = 0; i < model.meshCount; i++)
        {
            Mesh& mesh = model.meshes[i];

            Material& mat = model.materials[model.meshMaterial[i]];

            Color diffuseColor = mat.maps[MATERIAL_MAP_ALBEDO].color;

            if (mesh.colors == nullptr)
            {
                rlEnableVertexArray(mesh.vaoId);

                mesh.colors = (uint8_t*)MemAlloc(4 * mesh.vertexCount);

                mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] = 0;        // Vertex buffer: colors
                // Enable vertex attribute: color (shader-location = 3)
                mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] = rlLoadVertexBuffer(mesh.colors, mesh.vertexCount * 4 * sizeof(unsigned char), true);
                rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR);
            }

            for (int c = 0; c < mesh.vertexCount; c++)
            {
                mesh.colors[c * 4 + 0] = diffuseColor.r;
                mesh.colors[c * 4 + 1] = diffuseColor.g;
                mesh.colors[c * 4 + 2] = diffuseColor.b;
                mesh.colors[c * 4 + 3] = diffuseColor.a;
            }

            UpdateMeshBuffer(mesh, RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, mesh.colors, 4 * mesh.vertexCount, 0);
        }

        for (int i = 0; i < model.materialCount; i++)
        {
            model.materials[i].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        }
    }

    void CombineIdenticalMaterials()
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        std::map<int, Material> matLib;
        for (int i = 0; i < model.materialCount; i++)
        {
            matLib.insert_or_assign(i, model.materials[i]);
        }

        for (auto itr = matLib.begin(); itr != matLib.end(); itr++)
        {
            Material& thisMat = itr->second;

            auto otherItr = itr;
            otherItr++;

            // find all the dupes
            std::vector<std::map<int, Material>::iterator> dupes;
            for (; otherItr != matLib.end(); otherItr++)
            {
                Material& thatMat = otherItr->second;

                if (IsSameMaterial(thisMat, thatMat))
                {
                    dupes.push_back(otherItr);
                }
            }
            
            // merge the dupes into the current material
            for (auto& dupeItr : dupes)
            {
                for (int i = 0; i < model.meshCount; i++)
                {
                    if (model.meshMaterial[i] == dupeItr->first)
                        model.meshMaterial[i] = itr->first;
                }

                // delete the dupe
                matLib.erase(matLib.find(dupeItr->first));
            }
        }

        // collapse the materials down to the minimum list
        MemFree(model.materials);
        model.materials = (Material*)MemAlloc((unsigned int)(sizeof(Material) * matLib.size()));
        model.materialCount = int(matLib.size());

        int count = 0;
        for (const auto& [id, mat] : matLib)
        {
            for (int i = 0; i < model.meshCount; i++)
            {
                if (model.meshMaterial[i] == id)
                    model.meshMaterial[i] = count;
            }
            model.materials[count] = mat;
            count++;
        }
    }

    void MergeMeshes()
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        Model newModel = {0};
        
        newModel.meshCount = model.materialCount;
        newModel.meshMaterial = (int*)MemAlloc(sizeof(int) * newModel.meshCount);

        newModel.meshes = (Mesh*)MemAlloc(sizeof(Mesh) * newModel.meshCount);

        newModel.materialCount = model.materialCount;
        newModel.materials = model.materials;

        std::map<int, Mesh> meshes;
        for (int i = 0; i < model.meshCount; i++)
        {
            meshes.insert_or_assign(i, model.meshes[i]);
        }

        int newMeshIndex = 0;

        for (auto itr = meshes.begin(); itr != meshes.end(); itr++)
        {
            Mesh& thisMesh = itr->second;

            auto otherItr = itr;
            otherItr++;
            int triangleCount = itr->second.triangleCount;
            int vertexCount = itr->second.vertexCount;

            bool useIndexes = itr->second.indices != nullptr;

            // find all the dupes
            std::vector<std::map<int, Mesh>::iterator> dupes;
            for (; otherItr != meshes.end(); otherItr++)
            {
                Mesh& thatMesh = otherItr->second;

                if (model.meshMaterial[itr->first] == model.meshMaterial[otherItr->first])
                {
                    dupes.push_back(otherItr);

                    triangleCount += otherItr->second.triangleCount;
                    vertexCount += otherItr->second.vertexCount;
                }
            }

            newModel.meshes[newMeshIndex].vertexCount = vertexCount;
            newModel.meshes[newMeshIndex].triangleCount = triangleCount;

            newModel.meshes[newMeshIndex].vertices = (float*)MemAlloc(sizeof(float) * 3 * vertexCount);
            newModel.meshes[newMeshIndex].normals = (float*)MemAlloc(sizeof(float) * 3 * vertexCount);
            newModel.meshes[newMeshIndex].texcoords = (float*)MemAlloc(sizeof(float) * 2 * vertexCount);
            newModel.meshes[newMeshIndex].colors = (uint8_t*)MemAlloc(sizeof(uint8_t) * 4 * vertexCount);

            if (useIndexes)
                newModel.meshes[newMeshIndex].indices = (uint16_t*)MemAlloc(sizeof(uint16_t) * 3 * triangleCount);

            size_t vertOffset = 0;
            size_t normalOffset = 0;
            size_t textureCoordOffset = 0;
            size_t colorOffset = 0;
            size_t indexOffset = 0;

            // merge the dupes into the current material
            for (auto& dupeItr : dupes)
            {
                memcpy(newModel.meshes->vertices + vertOffset, dupeItr->second.vertices, dupeItr->second.vertexCount * sizeof(float) * 3);
                vertOffset += dupeItr->second.vertexCount * 3;

                memcpy(newModel.meshes->normals + normalOffset, dupeItr->second.normals, dupeItr->second.vertexCount * sizeof(float) * 3);
                normalOffset += dupeItr->second.vertexCount * 3;

                memcpy(newModel.meshes->texcoords + textureCoordOffset, dupeItr->second.texcoords, dupeItr->second.vertexCount * sizeof(float) * 2);
                textureCoordOffset += dupeItr->second.vertexCount * 2;

                memcpy(newModel.meshes->colors + colorOffset, dupeItr->second.colors, dupeItr->second.vertexCount * sizeof(uint8_t) * 4);
                colorOffset += dupeItr->second.vertexCount * 4;

                if (useIndexes)
                {
                    for (int index = 0; index < dupeItr->second.triangleCount * 3; index++)
                    {
                        newModel.meshes->indices[indexOffset + index] = (uint16_t)(dupeItr->second.indices[index] + indexOffset);
                    }

                    indexOffset += dupeItr->second.triangleCount * 3;
                }

                UploadMesh(newModel.meshes + newMeshIndex, true);

                // delete the dupe
                meshes.erase(meshes.find(dupeItr->first));

                newModel.meshMaterial[newMeshIndex] = model.meshMaterial[itr->first];
            }

            newMeshIndex++;
        }
    
        model.materialCount = 0;
        model.materials = nullptr;

        UnloadModel(model);

        App::SetModel(newModel);
        App::SetSeletedMesh(-1);
    }

    void ScaleMeshes(Vector3 scale)
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        for (size_t meshIndex = 0; meshIndex < App::GetModel().meshCount; meshIndex++)
        {
            Mesh& mesh = App::GetModel().meshes[meshIndex];
            for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
            {
                mesh.vertices[vertIndex * 3 + 0] *= scale.x;
                mesh.vertices[vertIndex * 3 + 1] *= scale.y;
                mesh.vertices[vertIndex * 3 + 2] *= scale.z;
            }

            UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
        }

        for (int i = 0; i < model.boneCount; i++)
        {
            model.bindPose[i].translation *= scale;
        }

        for (auto anim : App::GetAnimations().Animations)
        {
            for (int f = 0; f < anim->frameCount; f++)
            {
                for (int i = 0; i < model.boneCount; i++)
                {
                    anim->framePoses[f][i].translation *= scale;
                }
            }
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    void ScaleZTo(float z)
    {
        auto& model = App::GetModel();
        if (!IsModelValid(model))
            return;

        BoundingBox bbox = GetModelBoundingBox(model);

        if (bbox.max.z < z)
            return;

        float scale = z / bbox.max.z;

        ScaleMeshes(Vector3{ scale, scale, scale });
    }
}

TransformPanel::TransformPanel()
{
    Name = "Transformations";
    Icon = ICON_FA_UP_DOWN_LEFT_RIGHT;
    MinSize = { 200, 300 };

    DockingType = PanelDockingType::LeftDock;

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

    if (ImGui::Button("Combine Materials"))
        TransformTools::CombineIdenticalMaterials();

    if (ImGui::Button("Bake Material Colors"))
        TransformTools::BakeMaterialColors();
    
    if (ImGui::Button("Merge"))
        TransformTools::MergeMeshes();

    ImGui::TextUnformatted("Scale Z To");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::InputFloat("###ScaleTo", &ZSize);
    ImGui::SameLine();
    if (ImGui::Button("Apply"))
        TransformTools::ScaleZTo(ZSize);
    
}