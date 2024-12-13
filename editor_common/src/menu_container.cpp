#include "menu_container.h"

#include "raylib.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace Menus
{
    void MenuBar::Show()
    {
        if (!ImGui::BeginMainMenuBar())
            return;

        for (auto& menu : Contents)
        {
            if (menu->GetItemType() == Commands::CommandItemType::Item || !menu->IsActive())
                continue;

            ShowSubmenu((Commands::CommandContainer*)menu.get());
        }

        ImGui::EndMainMenuBar();
    }

    void MenuBar::ShowSubmenu(Commands::CommandContainer* container)
    {
        if (container->IsEmpty())
        {
            ImGui::MenuItem(container->GetName().data());
            return;
        }

        if (!ImGui::BeginMenu(container->GetName().data()))
            return;

        bool first = true;
        for (auto& item : container->Contents)
        {
            if (!item->IsActive())
                continue;

            switch (item->GetItemType())   
            {
            case Commands::CommandItemType::Item:
                ShowMenuItem((Commands::Command*)item.get());
                break;

            case Commands::CommandItemType::Container:
                ShowSubmenu((Commands::CommandContainer*)item.get());
                break;

            case Commands::CommandItemType::Group:
                ShowGroup((Commands::CommandContainer*)item.get(), first);
                break;
            default:
                break;
            }
            first = false;
        }

        ImGui::EndMenu();
    }

    void MenuBar::ShowGroup(Commands::CommandContainer* container, bool first)
    {
        if (!first)
            ImGui::Separator();

        for (auto& item : container->Contents)
        {
            if (!item->IsActive())
                continue;

            switch (item->GetItemType())
            {
            case Commands::CommandItemType::Item:
                ShowMenuItem((Commands::Command*)item.get());
                break;

            case Commands::CommandItemType::Container:
                ShowSubmenu((Commands::CommandContainer*)item.get());
                break;

            case Commands::CommandItemType::Group:
                ShowGroup((Commands::CommandContainer*)item.get(), false);
                break;
            default:
                break;
            }
        }
    }

    void MenuBar::ShowMenuItem(Commands::Command* command)
    {
        if (command == nullptr)
            return;

        bool selected = false;
        if (command->GetCommandType() == Commands::CommandType::ToggleTrigger)
            selected = command->GetValueF() != 0;

        const char* shortcut = nullptr;
        if (command->GetShortcut() != 0)
            shortcut = ImGui::GetKeyChordName(command->GetShortcut());

        std::string_view icon = command->GetIcon();
        if (icon.empty())
            icon = " ";

        const char* name = TextFormat("%s  %s\t", icon.data(), command->GetName().data());

        if (ImGui::MenuItem(name, shortcut, selected, command->GetEnabled()))
        {
            command->Trigger();
        }
    }

    void MenuBar::ProcessShortcuts()
    {
        for (auto& menu : Contents)
        {
            if (menu->GetItemType() == Commands::CommandItemType::Item)
                continue;

            ProcessShortcuts(menu.get());
        }
    }

    void MenuBar::ProcessShortcuts(Commands::CommandItem* item)
    {
        if (!item->IsActive())
            return;

        switch (item->GetItemType())
        {
        case Commands::CommandItemType::Item:
        {
            Commands::Command* command = (Commands::Command*)item;
            ImGuiKeyChord shortcut = command->GetShortcut();
            if (shortcut != 0 && ImGui::IsKeyChordPressed(command->GetShortcut()))
                command->Trigger();
        }
            break;

        case Commands::CommandItemType::Container:
        case Commands::CommandItemType::Group:
        {
            Commands::CommandContainer* container = (Commands::CommandContainer*)item;
            for (auto& childItem : container->Contents)
                ProcessShortcuts(childItem.get());
        }
            break;

        default:
            break;
        }
    }

}
