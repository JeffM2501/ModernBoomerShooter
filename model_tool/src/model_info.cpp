#include "model_info.h"

#include "extras/IconsFontAwesome6.h"

#include "imgui.h"

namespace App
{
    extern bool ShowBones;
}

ModelInfoPanel::ModelInfoPanel()
{
    Name = "Model Info";
    Icon = ICON_FA_CUBE;

    MinSize = { 250, 300 };

    DockingType = PanelDockingType::RightDock;
}

void ModelInfoPanel::OnShowContents()
{
    Model& model = App::GetModel();
    if (!IsModelValid(model))
    {
        ImGui::TextUnformatted("No Model Loaded");
        return;
    }

    if (ImGui::BeginChild("Meshes", ImVec2(0, 200)))
    {
        ImGui::Text("Meshes %d", model.meshCount);
        if (ImGui::BeginChild("MeshList", ImVec2(0, 175), ImGuiChildFlags_Borders))
        {
            for (int i = 0; i < model.meshCount; i++)
            {
                bool selected = App::GetSelectedMesh() == i;
                if (ImGui::Selectable(TextFormat("Mesh %2d Mat %2d", i, model.meshMaterial[i]), &selected, ImGuiSelectableFlags_None))
                {
                    if (selected)
                        App::SetSeletedMesh(i);
                }
            }
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }

    if (ImGui::BeginChild("Materials", ImVec2(0, 100)))
    {
        ImGui::Text("Materials %d", model.materialCount);

        if (ImGui::BeginChild("MatList", ImVec2(0, 75), ImGuiChildFlags_Borders))
        {
            for (int i = 0; i < model.materialCount; i++)
            {
                bool selected = false;
                ImGui::Selectable(TextFormat("Material %d w%d h%d", i, model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture.width, model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture.height), &selected, ImGuiSelectableFlags_None);
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }

    if (ImGui::BeginChild("Bones", ImVec2(0, 150)))
    {
        ImGui::Text("Bones %d", model.boneCount);

        if (ImGui::BeginChild("BoneList", ImVec2(0, 100), ImGuiChildFlags_Borders))
        {
            for (int i = 0; i < model.boneCount; i++)
            {
                bool selected = App::GetSelectedBone() == i;
                if (ImGui::Selectable(TextFormat("Bone %d (%s)", i, model.bones[i].name), &selected, ImGuiSelectableFlags_None))
                {
                    if (selected)
                        App::SetSeletedBone(i);
                }
            }
            ImGui::EndChild();
        }

        if (model.boneCount > 0)
        {
            ImGui::Checkbox("Show Bones", &App::ShowBones);
        }
        ImGui::EndChild();
    }

    auto& anims = App::GetAnimations();

    if (!anims.Animations.empty() && ImGui::BeginChild("Animations", ImVec2(0, 250)))
    {
        ImGui::Text("Animations %d", int(anims.Animations.size()));

        if (ImGui::BeginChild("AnimList", ImVec2(0, 200), ImGuiChildFlags_Borders))
        {
            int count = 0;
            for (auto * animSequence : anims.Animations)
            {
                bool selected = count == anims.Sequence;
                if (ImGui::Selectable(TextFormat("%s", animSequence->name), &selected, ImGuiSelectableFlags_None))
                {
                    if (anims.Sequence != count)
                    {
                        anims.Sequence = count;
                        anims.Frame = 0;
                        App::RebuildAnimFrame();
                    }
                }
                count++;
            }
            ImGui::EndChild();
        }

        if (model.boneCount > 0 && anims.Sequence >= 0)
        {
            if (ImGui::SliderInt("Frame", &anims.Frame, 0, anims.Animations[anims.Sequence]->frameCount - 1))
                App::RebuildAnimFrame();
        }
        ImGui::EndChild();
    }
}