#include "app.h"

#include "rlImGui.h"
#include "imgui.h"

#include "menu_container.h"
#include "tinyfiledialogs.h"

#include "transform_tools.h"
#include "model_info.h"

#include <vector>

namespace App
{
    static constexpr char WindowSection[] = "WindowSection";
    static constexpr char DocumentSection[] = "DocumentSection";
    static constexpr char End[] = "End";

    char const* lFilterPatterns[2] = { "*.glb", "*.gltf" };

    Menus::MenuBar MainMenu;

    std::vector<std::unique_ptr<Panel>> Panels;
    void AddPanel(std::unique_ptr<Panel> panel)
    {
        Panels.emplace_back(std::move(panel));
    }

    void OpenModel()
    {
        const char* fileName = tinyfd_openFileDialog(
            "Open Mesh",
            "../*.glb",
            2,
            lFilterPatterns,
            "*.glb|*.gltf",
            1);

            if (fileName != nullptr)
                LoadModel(fileName);
    }

    void SetupMenu()
    {
        Menus::SubMenu& fileMenu = MainMenu.Add<Menus::SubMenu>("File");

        fileMenu.Add<Commands::SimpleTriggerCommand>("Open"
            , ICON_FA_UPLOAD
            , "Open File"
            , ImGuiKey_O | ImGuiKey_ModCtrl
            , []() { OpenModel(); });

        auto& exitGroup = fileMenu.Add<Commands::CommandContainer>("Exit");

        exitGroup.Add<Commands::SimpleTriggerCommand>("Exit"
            , ICON_FA_DOOR_OPEN
            , "Exit Application"
            , ImGuiKey_F4 | ImGuiKey_ModAlt
            , []() {RequestQuit(); });

        MainMenu.Add<Menus::MenuPlaceHolder>(DocumentSection);
        MainMenu.Add<Menus::MenuPlaceHolder>(WindowSection);
        Menus::SubMenu& windowMenu = MainMenu.Add<Menus::SubMenu>("Windows");

        for (auto& panel : Panels)
        {
            auto* panelPtr = panel.get();

            windowMenu.Add<Commands::SimpleToggleTriggerCommand>(panel->Name.c_str()
                , panel->Icon.c_str()
                , panel->Name.c_str()
                , ImGuiKey_None
                , [panelPtr]() { panelPtr->Open = !panelPtr->Open; }
            , [panelPtr]() {return panelPtr->Open; });
        }
        MainMenu.Add<Menus::MenuPlaceHolder>(End);
    }

    void InitGui()
    {
        rlImGuiSetup(true);

        AddPanel<TransformPanel>();
        AddPanel<ModelInfoPanel>();

        SetupMenu();
    }

    void ShowGui()
    {
        rlImGuiBegin();
        MainMenu.Show();

        MainMenu.ProcessShortcuts();
        for (auto& panel : Panels)
        {
            panel->Show();
        }

        rlImGuiEnd();
    }

    void CleanupGui()
    {
        rlImGuiShutdown();
    }
}

void Panel::Show()
{
    if (!Open)
        return;

    const char* uniqueName = TextFormat("%s  %s###%s_panel", Icon.c_str(), Name.c_str(), Name.c_str());

    ImGuiWindowFlags flags = ImGuiWindowFlags_None | ExtraWindowFlags;

    float windowStartY = App::MainMenu.Height + ImGui::GetTextLineHeight();

    float windowEndY = GetScreenHeight() - windowStartY - ImGui::GetTextLineHeight();

    switch (DockingType)
    {
    case PanelDockingType::Floating:
        if (MinSize.x > 0 && MinSize.y > 0)
            ImGui::SetNextWindowSizeConstraints(ImVec2(MinSize.x, MinSize.y), ImVec2(float(GetScreenWidth()), float(GetScreenHeight())));
        break;

    case PanelDockingType::LeftDock:
        if (MinSize.x < 0)
            MinSize.x = 150;

        ImGui::SetNextWindowPos(ImVec2(0, windowStartY));
        ImGui::SetNextWindowSize(ImVec2(MinSize.x, windowEndY));

        flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        break;

    case PanelDockingType::RightDock:
        if (MinSize.x < 0)
            MinSize.x = 150;

        ImGui::SetNextWindowPos(ImVec2(GetScreenWidth()-MinSize.x, windowStartY));
        ImGui::SetNextWindowSize(ImVec2(MinSize.x, windowEndY));

        flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
        break;
    }

    if (ImGui::Begin(Name.c_str(), &Open, flags))
    {
        OnShowContents();
    }
    ImGui::End();
}
