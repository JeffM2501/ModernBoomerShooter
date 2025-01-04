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
    auto& model = App::GetModel();
    if (model.Groups.empty())
    {
        ImGui::TextUnformatted("No Model Loaded");
        return;
    }

    if (ImGui::BeginChild("Meshes", ImVec2(0, 200)))
    {
        ImGui::Text("Meshes");
        if (ImGui::BeginChild("MeshList", ImVec2(0, 175), ImGuiChildFlags_Borders))
        {
            int i = 0;
            for (auto& group : model.Groups)
            {
                for (auto& mesh : group.Meshes)
                {
                    bool selected = App::GetSelectedMesh() == i;
                    if (ImGui::Selectable(TextFormat("Mesh %2d", i), &selected, ImGuiSelectableFlags_None))
                    {
                        if (selected)
                            App::SetSeletedMesh(i);
                    }

                    i++;
                }
            }
            ImGui::EndChild();
        }

        ImGui::EndChild();
    }

    auto& anims = App::GetAnimations();

    if (!anims.Animations.Sequences.empty() && ImGui::BeginChild("Animations", ImVec2(0, 250)))
    {
        ImGui::Text("Animations %d", int(anims.Animations.Sequences.size()));

        if (ImGui::BeginChild("AnimList", ImVec2(0, 200), ImGuiChildFlags_Borders))
        {
            int count = 0;
            for (auto & [name, sequence] : anims.Animations.Sequences)
            {
                bool selected = name == anims.Sequence;
                if (ImGui::Selectable(TextFormat("%s", name.c_str()), &selected, ImGuiSelectableFlags_None))
                {
                    if (name != anims.Sequence)
                    {
                        anims.Sequence = name;
                        anims.Frame = 0;
                    }
                }
                count++;
            }
            ImGui::EndChild();
        }

        if (!anims.Sequence.empty())
        {
            ImGui::SliderInt("Frame", &anims.Frame, 0, int(anims.Animations.Sequences[anims.Sequence].Frames.size()) - 1);
        }
        ImGui::EndChild();
    }
}