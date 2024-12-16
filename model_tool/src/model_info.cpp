#include "model_info.h"

#include "extras/IconsFontAwesome6.h"

#include "imgui.h"

ModelInfoPanel::ModelInfoPanel()
{
    Name = "Model Info";
    Icon = ICON_FA_CUBE;

    MinSize = { 150, 300 };

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

    ImGui::Text("Meshes %d", model.meshCount);
    ImGui::Separator();
    for (int i = 0; i < model.meshCount; i++)
    {
        bool selected = false;
        ImGui::Selectable(TextFormat("Mesh %2d Mat %2d", i, model.meshMaterial[i]), &selected, ImGuiSelectableFlags_None);
    }
    ImGui::Separator();
    ImGui::Text("Materials %d", model.materialCount);
    ImGui::Separator();

    for (int i = 0; i < model.materialCount; i++)
    {
        bool selected = false;
        ImGui::Selectable(TextFormat("Material %d w%d h%d", i, model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture.width, model.materials[i].maps[MATERIAL_MAP_ALBEDO].texture.height), &selected, ImGuiSelectableFlags_None);
    }
}