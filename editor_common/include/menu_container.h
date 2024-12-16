#pragma once

#include "command_system.h"

using namespace Commands;

namespace Menus
{
    class MenuBar : public CommandContainer
    {
    public:
        void Show();

        void ProcessShortcuts();

        float Height = 0;

    protected:
        void ShowSubmenu(CommandContainer* container);
        void ShowGroup(CommandContainer* container, bool first);
        void ShowMenuItem(Command* command);

        void ProcessShortcuts(CommandItem* command);
    };

    class MenuPlaceHolder : public CommandItem
    {
    private:
        std::string Name;
    public:
        MenuPlaceHolder(std::string_view name) : Name(name) {}
        CommandItemType GetItemType() const override { return CommandItemType::None; }
        std::string_view GetName() override  { return Name; }
        bool IsActive() const { return false; }
    };

    class SubMenu : public CommandContainer
    {
    public:
        SubMenu() = default;
        SubMenu(std::string_view name) : CommandContainer(name) {}

        CommandItemType GetItemType() const override { return CommandItemType::Container; }
    };
}