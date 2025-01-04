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
#include <functional>

namespace TransformTools
{

    void DoForEachMesh(Models::AnimateableModel& model, std::function<void(Mesh&)> func)
    {
        for (auto& group : model.Groups)
        {
            for (auto& mesh : group.Meshes)
            {
                func(mesh.Geometry);
            }
        }
    }

    void CenterMesh()
    {
        auto& model = App::GetModel();
        if (model.Groups.empty())
            return;

        BoundingBox bbox = model.GetBounds();

        Vector3 center = (bbox.max - bbox.min) / 2 + bbox.min;

        DoForEachMesh(model, [center](Mesh& mesh)
            {
                for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
                {
                    mesh.vertices[vertIndex * 3 + 0] -= center.x;
                    mesh.vertices[vertIndex * 3 + 1] -= center.y;
                    mesh.vertices[vertIndex * 3 + 2] -= center.z;
                }

                UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
            });

        for (auto &bone : model.Bones)
        {
            bone.DefaultGlobalTransform.translation -= center;
        }

        for (auto & [name, sequence] : App::GetAnimations().Animations.Sequences)
        {
            for (auto& keyframe : sequence.Frames)
            {
                for (auto & transform : keyframe.GlobalTransforms)
                {
                    transform.translation -= center;
                }
            }
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    void FloorMesh()
    {
        auto& model = App::GetModel();
        if (model.Groups.empty())
            return;

        BoundingBox bbox = model.GetBounds();

        DoForEachMesh(model, [bbox](Mesh& mesh)
            {
                for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
                {
                    mesh.vertices[vertIndex * 3 + 1] -= bbox.min.y;
                }

                UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
            });

        for (auto& bone : model.Bones)
        {
            bone.DefaultGlobalTransform.translation.y -= bbox.min.y;
        }

        for (auto& [name, sequence] : App::GetAnimations().Animations.Sequences)
        {
            for (auto& keyframe : sequence.Frames)
            {
                for (auto& transform : keyframe.GlobalTransforms)
                {
                    transform.translation.y -= bbox.min.y;
                }
            }
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    void RotateMesh(float angle, Vector3 axis)
    {
        Matrix mat = MatrixRotate(axis, angle * DEG2RAD);
        Quaternion rot = QuaternionFromAxisAngle(axis, angle * DEG2RAD);

        auto& model = App::GetModel();
        if (model.Groups.empty())
            return;

        model.RootTransform = MatrixRotate(axis, angle * DEG2RAD);
        return;
    }

    void ScaleMeshes(Vector3 scale)
    {
        auto& model = App::GetModel();
        if (model.Groups.empty())
            return;

        DoForEachMesh(model, [scale](Mesh& mesh)
            {
                for (size_t vertIndex = 0; vertIndex < mesh.vertexCount; vertIndex++)
                {
                    mesh.vertices[vertIndex * 3 + 0] *= scale.x;
                    mesh.vertices[vertIndex * 3 + 1] *= scale.y;
                    mesh.vertices[vertIndex * 3 + 2] *= scale.z;
                }

                UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * sizeof(float), 0);
            });

        for (auto& bone : model.Bones)
        {
            bone.DefaultGlobalTransform.translation *= scale;

        }

        for (auto& [name, sequence] : App::GetAnimations().Animations.Sequences)
        {
            for (auto& keyframe : sequence.Frames)
            {
                for (auto& transform : keyframe.GlobalTransforms)
                {
                    transform.translation *= scale;
                }
            }
        }

        App::SetSeletedMesh(App::GetSelectedMesh());
    }

    void ScaleZTo(float z)
    {
        auto& model = App::GetModel();
        if (model.Groups.empty())
            return;

        BoundingBox bbox = model.GetBounds();

        if (bbox.max.z < z)
            return;

        float scale = z / bbox.max.z;

        ScaleMeshes(Vector3{ scale, scale, scale });
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
        if (model.Groups.empty())
            return;

        for (auto & group : model.Groups)
        {
            Material& mat = group.GroupMaterial;
            for (auto& mesh : group.Meshes)
            {
                Color diffuseColor = mat.maps[MATERIAL_MAP_ALBEDO].color;

                if (mesh.Geometry.colors == nullptr)
                {
                    rlEnableVertexArray(mesh.Geometry.vaoId);

                    mesh.Geometry.colors = (uint8_t*)MemAlloc(4 * mesh.Geometry.vertexCount);

                    mesh.Geometry.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] = 0;        // Vertex buffer: colors
                    // Enable vertex attribute: color (shader-location = 3)
                    mesh.Geometry.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR] = rlLoadVertexBuffer(mesh.Geometry.colors, mesh.Geometry.vertexCount * 4 * sizeof(unsigned char), true);
                    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR);
                }

                for (int c = 0; c < mesh.Geometry.vertexCount; c++)
                {
                    mesh.Geometry.colors[c * 4 + 0] = diffuseColor.r;
                    mesh.Geometry.colors[c * 4 + 1] = diffuseColor.g;
                    mesh.Geometry.colors[c * 4 + 2] = diffuseColor.b;
                    mesh.Geometry.colors[c * 4 + 3] = diffuseColor.a;
                }

                UpdateMeshBuffer(mesh.Geometry, RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, mesh.Geometry.colors, 4 * mesh.Geometry.vertexCount, 0);
            }

            mat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
        }
    }

    void CombineIdenticalMaterials()
    {
        auto& model = App::GetModel();
        if (model.Groups.empty())
            return;


        for (size_t groupId = 0; groupId < model.Groups.size(); groupId++)
        {
            for (size_t otherGroupId = 0; otherGroupId < model.Groups.size(); otherGroupId++)
            {
                if (groupId == otherGroupId)
                    continue;

                auto& group = model.Groups[groupId];
                auto& otherGroup = model.Groups[otherGroupId];

                if (group.Meshes.empty() || otherGroup.Meshes.empty())
                    continue;

                if (IsSameMaterial(group.GroupMaterial, otherGroup.GroupMaterial))
                {
                    group.Meshes.insert(group.Meshes.end(), otherGroup.Meshes.begin(), otherGroup.Meshes.end());
                    otherGroup.Meshes.clear();
                }
            }
        }

        for (auto itr = model.Groups.begin(); itr != model.Groups.end();)
        {
            if (itr->Meshes.empty())
            {
                MemFree(itr->GroupMaterial.maps);
                itr = model.Groups.erase(itr);
            }
            else
            {
                itr++;
            }
        }

        /* std::map<int, Material> matLib;
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
        */
    }

    void MergeMeshes()
    {
        /*
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
                // delete the dupe
                meshes.erase(meshes.find(dupeItr->first));

                newModel.meshMaterial[newMeshIndex] = model.meshMaterial[itr->first];
            }

            UploadMesh(newModel.meshes + newMeshIndex, true);
            newMeshIndex++;
        }
    
        model.materialCount = 0;
        model.materials = nullptr;

        UnloadModel(model);

        App::SetModel(newModel);
        App::SetSeletedMesh(-1);
        */
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
    float spacing = 2;
    if (ImGui::Button(ICON_FA_BORDER_NONE))
        TransformTools::CenterMesh();
    ImGui::SetItemTooltip("Center");

    ImGui::SameLine(0, spacing);
    if (ImGui::Button(ICON_FA_ARROWS_DOWN_TO_LINE))
        TransformTools::FloorMesh();
    ImGui::SetItemTooltip("Floor");
    
    ImGui::SameLine(0, spacing);
    if (ImGui::Button("Z" ICON_FA_ARROW_UP))
        TransformTools::RotateMesh(90, Vector3UnitX);
    ImGui::SetItemTooltip("Set Z Up");
    
    ImGui::SameLine(0, spacing);
    if (ImGui::Button(ICON_FA_DROPLET_SLASH))
        TransformTools::BakeMaterialColors();
    ImGui::SetItemTooltip("Bake Material Colors");

    ImGui::SameLine(0, spacing);
    if (ImGui::Button(ICON_FA_OBJECT_GROUP))
        TransformTools::CombineIdenticalMaterials();
    ImGui::SetItemTooltip("Combine Identical Materials");

    ImGui::TextUnformatted("Fit Y");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::InputFloat("###ScaleTo", &ZSize);
    ImGui::SameLine();
    if (ImGui::Button("Apply"))
        TransformTools::ScaleZTo(ZSize);
    
}