#include "systems/input_system.h"
#include "systems/console_render_system.h"
#include "services/global_vars.h"

#include "raylib.h"
#include "world.h"

AxisActionDef::AxisActionDef(KeyboardKey positive, KeyboardKey negative, int mouseAxis, float mouseAxisScale, int gamepadAxis, float gamepadAxisScale)
{
    Type = ActionType::Axis;
    if (positive != KEY_NULL)
        PositiveKeys.push_back(positive);

    if (negative != KEY_NULL)
        NegativeKeys.push_back(negative);
    GamePadAxis = gamepadAxis;
    AxisScale = gamepadAxisScale;

    MouseAxis = mouseAxis;
    MouseAxisScale = mouseAxisScale;
}

void AxisActionDef::Update(bool allowKeyboard)
{
    if (GlobalVars::Paused)
        return;

    AxisValue = 0;

    float posValue = 0;
    float negValue = 0;

    if (allowKeyboard)
    {
        for (auto& postive : PositiveKeys)
        {
            if (IsKeyDown(postive))
            {
                AxisValue += 1;
                break;
            }
        }

        for (auto& negative : NegativeKeys)
        {
            if (negative != KEY_NULL && IsKeyDown(negative))
            {
                AxisValue += -1;
                break;
            }
        }
    }

    if (IsGamepadAvailable(0))
    {
        float axisValue = 0;
        if (GamePadAxis >= 0)
        {
            axisValue = GetGamepadAxisMovement(0, GamePadAxis) * AxisScale;
        }

        AxisValue += axisValue;

        if (AxisValue != 0)
            AxisValue = AxisValue / fabsf(AxisValue);
    }

    if (MouseAxis >= 0)
    {
        if (MouseAxis == 0 && fabsf(GetMouseDelta().x) > 0)
            AxisValue = GetMouseDelta().x * MouseAxisScale;
        else if (MouseAxis == 1 && fabsf(GetMouseDelta().y) > 0)
            AxisValue = GetMouseDelta().y * MouseAxisScale;
    }
}

float AxisActionDef::GetValue() const
{
    return AxisValue;
}

bool AxisActionDef::IsActive() const
{
    return AxisValue != 0;
}

ButtonActionDef::ButtonActionDef(KeyboardKey key, int mouseButton, GamepadButton button)
{
    Type = ActionType::Button;
    ButtonKeys.push_back(key);
    PadButton = button;
    MouseButton = mouseButton;
}

void ButtonActionDef::Update(bool allowKeyboard)
{
    ButtonValue = false;

    if (allowKeyboard)
    {
        for (auto& postive : ButtonKeys)
        {
            if (IsKeyDown(postive))
            {
                ButtonValue = true;
                break;
            }
        }
    }

    if (IsGamepadAvailable(0))
    {
        if (PadButton != GAMEPAD_BUTTON_UNKNOWN)
        {
            if (IsGamepadButtonDown(0, PadButton))
                ButtonValue = true;
        }
    }

    if (MouseButton >= 0)
    {
        if (IsMouseButtonDown(MouseButton))
            ButtonValue = true;
    }
}

float ButtonActionDef::GetValue() const
{
    return ButtonValue ? 1.0f : 0.0f;
}

bool ButtonActionDef::IsActive() const
{
    return ButtonValue;
}

CommandActionDef::CommandActionDef(KeyboardKey key, std::string_view command, ConsoleRenderSystem* console)
{
    Type = ActionType::Command;
    ConsoleSystem = console;
    Command = command;
    ActionKeys.push_back(key);
}

void CommandActionDef::Update(bool allowKeyboard)
{
    if (!Command.empty() && ConsoleSystem)
    {
        for (auto& postive : ActionKeys)
        {
            if (IsKeyPressed(postive))
            {
                ConsoleSystem->ProcessCommand(Command);
                return;
            }
        }
    }
}

ButtonActionDef* InputSystem::AddButtonAction(uint8_t id, KeyboardKey key, int mouseButton, GamepadButton button)
{
    auto itr = Actions.find(id);
    if (itr != Actions.end())
    {
        if (itr->second->Type != ActionType::Button)
            return nullptr;

        return static_cast<ButtonActionDef*>(itr->second.get());
    }

    return static_cast<ButtonActionDef*>(Actions.try_emplace(id, std::make_unique<ButtonActionDef>(key, mouseButton, button)).first->second.get());
}

AxisActionDef* InputSystem::AddAxisAction(uint8_t id, KeyboardKey positive, KeyboardKey negative, int mouseAxis, float mouseAxisScale, int gamepadAxis, float gamepadAxisScale)
{
    auto itr = Actions.find(id);
    if (itr != Actions.end())
    {
        if (itr->second->Type != ActionType::Axis)
            return nullptr;

        return static_cast<AxisActionDef*>(itr->second.get());
    }

    return static_cast<AxisActionDef*>(Actions.try_emplace(id, std::make_unique<AxisActionDef>(positive, negative, mouseAxis,mouseAxisScale, gamepadAxis, gamepadAxisScale)).first->second.get());
}

CommandActionDef* InputSystem::AddCommandAction(uint8_t id, KeyboardKey key, std::string_view command)
{
    auto itr = Actions.find(id);
    if (itr != Actions.end())
    {
        if (itr->second->Type != ActionType::Command)
            return nullptr;

        return static_cast<CommandActionDef*>(itr->second.get());
    }

    return static_cast<CommandActionDef*>(Actions.try_emplace(id, std::make_unique<CommandActionDef>(key, command, ConsoleSystem)).first->second.get());
}

ActionDef* InputSystem::GetAction(uint8_t id)
{
    auto itr = Actions.find(id);
    if (itr == Actions.end())
        return nullptr;

    return itr->second.get();
}

float InputSystem::GetActionValue(uint8_t id) const
{
    auto itr = Actions.find(id);
    if (itr == Actions.end())
        return 0;

    return itr->second->GetValue();
}

bool InputSystem::IsActionActive(uint8_t id) const
{
    auto itr = Actions.find(id);
    if (itr == Actions.end())
        return 0;

    return itr->second->IsActive();
}

void InputSystem::OnInit()
{
    AddAxisAction(Actions::Forward, KEY_W, KEY_S, -1, -1, GAMEPAD_AXIS_LEFT_Y);
    AddAxisAction(Actions::Sideways, KEY_D, KEY_A, -1, -1, GAMEPAD_AXIS_RIGHT_X);

    AddAxisAction(Actions::Yaw, KEY_NULL, KEY_NULL, 0, -0.25f, GAMEPAD_AXIS_LEFT_X, 100);
    AddAxisAction(Actions::Pitch, KEY_NULL, KEY_NULL, 1, 0.25f, GAMEPAD_AXIS_RIGHT_Y, 100);

    AddButtonAction(Actions::Fire, KEY_SPACE, 0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

void InputSystem::OnSetup()
{
    ConsoleSystem = WorldPtr->GetSystem<ConsoleRenderSystem>();

    AddCommandAction(Actions::Reload, KEY_F5, ConsoleCommands::Reload);
}

void InputSystem::OnUpdate()
{
    if (WindowShouldClose())
        WorldPtr->Quit();  

    bool allowKeys = true;
    if (ConsoleSystem)
        allowKeys = !ConsoleSystem->WantKeyInput();

    for (auto& [id, action] : Actions)
        action->Update(allowKeys);
}
