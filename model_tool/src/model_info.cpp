#include "model_info.h"

#include "extras/IconsFontAwesome6.h"

#include "imgui.h"

ModelInfoPanel::ModelInfoPanel()
{
    Name = "Model Info";
    Icon = ICON_FA_CUBE;

    MinSize = { 250,100 };

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
    ImGui::Text("Materials %d", model.materialCount);
}