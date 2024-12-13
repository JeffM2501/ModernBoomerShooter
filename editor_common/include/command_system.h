#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "imgui.h"
#include "extras/IconsFontAwesome6.h"

namespace Commands
{
    enum CommandItemType
    {
        None,
        Item,
        Group,
        Container,
    };

    class CommandItem
    {
    public:
        virtual CommandItemType GetItemType() const = 0;
        virtual std::string_view GetName() { return ""; }

        virtual bool IsActive() const { return true; }
    };

    enum class CommandType
    {
        Trigger,
        ToggleTrigger,
        Float,
    };

    class Command : public CommandItem
    {
    public:
        CommandItemType GetItemType() const final { return CommandItemType::Item; }

        virtual CommandType GetCommandType() { return CommandType::Trigger; }

        virtual void Trigger() {}
        virtual float GetValueF() { return 0; }
        virtual void SetValue(float value) {}

        virtual std::string_view GetIcon() { return ICON_FA_PUZZLE_PIECE; }
        virtual std::string_view GetDescription() { return ""; }

        virtual bool GetEnabled() { return false; }

        virtual ImGuiKeyChord GetShortcut() { return 0; }
    };

#define DEFINE_SIMPLE_TRIGGER_COMMAND(N, I, D, S) \
    std::string_view GetName() override { return N; } \
    std::string_view GetIcon() override { return I; } \
    std::string_view GetDescription() override { return D; } \
    ImGuiKeyChord GetShortcut() override { return S; } \
    bool GetEnabled() override { return true; }

    class SimpleTriggerCommand : public Command
    {
    private:
        std::string Name;
        std::string Icon;
        std::string Description;
        ImGuiKeyChord Shortcut;

        std::function<void()> TriggerFunc;
    public:

        SimpleTriggerCommand(std::string_view name, std::string_view icon, std::string_view description, ImGuiKeyChord shortcut, std::function<void()> func)
            : Name(name)
            , Icon(icon)
            , Description(description)
            , Shortcut(shortcut)
            , TriggerFunc(func)
        {}

        std::string_view GetName() override { return Name; } 
        std::string_view GetIcon() override { return Icon; }
        std::string_view GetDescription() override { return Description; }
        ImGuiKeyChord GetShortcut() override { return Shortcut; }
        bool GetEnabled() override { return true; }

        void Trigger() override
        {
            if (TriggerFunc)
                TriggerFunc();
        }
    };

    class SimpleToggleTriggerCommand : public Command
    {
    private:
        std::string Name;
        std::string Icon;
        std::string Description;
        ImGuiKeyChord Shortcut;

        std::function<void()> TriggerFunc;
        std::function<bool()> SelectedFunc;
        std::function<bool()> EnabledFunc;
    public:

        SimpleToggleTriggerCommand(std::string_view name, std::string_view icon, std::string_view description, ImGuiKeyChord shortcut, std::function<void()> trigger, std::function<bool()> selected = nullptr, std::function<bool()> enabled = nullptr)
            : Name(name)
            , Icon(icon)
            , Description(description)
            , Shortcut(shortcut)
            , TriggerFunc(trigger)
            , SelectedFunc(selected)
            , EnabledFunc(enabled)
        {}
        CommandType GetCommandType() override { return CommandType::ToggleTrigger; }

        std::string_view GetName() override { return Name; }
        std::string_view GetIcon() override { return Icon; }
        std::string_view GetDescription() override { return Description; }
        ImGuiKeyChord GetShortcut() override { return Shortcut; }

        void Trigger() override
        {
            if (TriggerFunc)
                TriggerFunc();
        }

        bool GetEnabled() override 
        {
            if (EnabledFunc)
                return EnabledFunc();

            return true;
        }

        float GetValueF()override
        {
            if (SelectedFunc)
                return SelectedFunc() ? 1.0f : 0.0f;
            return 0;
        }
    };

    class CommandContainer : public CommandItem
    {
    protected:
        std::string Name;

    public:
        using ItemVector = std::vector<std::unique_ptr<CommandItem>>;
        ItemVector Contents;

        CommandContainer() = default;
        CommandContainer(std::string_view name) : Name(name) {}

        CommandItemType GetItemType() const override { return CommandItemType::Group; }
        std::string_view GetName() override { return Name; }

        inline Commands::CommandContainer* Find(std::string_view name)
        {
            for (auto& item : Contents)
            {
                if (item->GetItemType() == CommandItemType::Item)
                    continue;

                if (item->GetName() == name)
                    return (Commands::CommandContainer*)(item.get());
            }

            return nullptr;
        }

        bool IsEmpty() const
        {
            for (auto& item : Contents)
            {
                if (item->GetItemType() != CommandItemType::Group)
                    return false;

                Commands::CommandContainer* container = (Commands::CommandContainer*)item.get();
                if (!container->IsEmpty())
                    return false;
            }
            return true;
        }

        template<class T, typename... Args>
        T& Add(Args&&... args)
        {
            Contents.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
            return *(T*)(Contents.back().get());
        }

        template<class T, typename... Args>
        T& Insert(std::string_view name, bool before, Args&&... args)
        {
            ItemVector::iterator itemItr = Contents.begin();
            for (; itemItr != Contents.end(); itemItr++)
            {
                if (itemItr->get()->GetName() == name)
                {
                    if (!before)
                        itemItr++;
                    break;
                }
            }

            std::unique_ptr<T> item = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = item.get();
            Contents.emplace(itemItr, std::move(item));

            return *ptr;
        }

        void Remove(CommandItem* item)
        {
            for (auto itr = Contents.begin(); itr != Contents.end(); itr++)
            {
                if (itr->get() == item)
                {
                    Contents.erase(itr);
                    return;
                }
            }
        }
    };
}