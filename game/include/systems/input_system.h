#pragma once

#include "system.h"
#include "raylib.h"

#include <memory>
#include <unordered_map>

namespace Actions
{
    static constexpr uint8_t Forward = 0;
    static constexpr uint8_t Sideways = 1;
    static constexpr uint8_t Fire = 2;
    static constexpr uint8_t Yaw = 3;
    static constexpr uint8_t Pitch = 4;
}

enum class ActionType
{
    Button,
    Axis,
};

struct ActionDef
{
    ActionType Type = ActionType::Axis;

    virtual void Update() = 0;

    virtual float GetValue() const = 0;
    virtual bool IsActive() const = 0;
};

struct AxisActionDef : public ActionDef
{
    std::vector<KeyboardKey> PositiveKeys;
    std::vector<KeyboardKey> NegativeKeys;

    int GamePadAxis = -1;
    float AxisScale = 1;

    int MouseAxis = -1;
    float MouseAxisScale = 1;

    AxisActionDef() = default;
    AxisActionDef(KeyboardKey positive, KeyboardKey negative, int mouseAxis = -1, float mouseAxisScale = 1, int gamepadAxis = -1, float gamepadAxisScale = 1);

    void Update() override;

    float GetValue() const override;
    bool IsActive() const override;

private:
    float AxisValue = 0;
};

struct ButtonActionDef : public ActionDef
{
    std::vector<KeyboardKey> ButtonKeys;
    GamepadButton PadButton = GAMEPAD_BUTTON_UNKNOWN;
    int MouseButton = -1;

    ButtonActionDef() = default;
    ButtonActionDef(KeyboardKey key, int mouseButton = -1, GamepadButton button = GAMEPAD_BUTTON_UNKNOWN);

    void Update() override;

    float GetValue() const override;
    bool IsActive() const override;

private:
    bool ButtonValue = false;
};

class InputSystem : public System
{
public:
    DEFINE_SYSTEM(InputSystem)

    ButtonActionDef* AddButtonAction(uint8_t id, KeyboardKey key = KEY_NULL, int mouseButton = -1, GamepadButton button = GAMEPAD_BUTTON_UNKNOWN);
    AxisActionDef* AddAxisAction(uint8_t id, KeyboardKey positive = KEY_NULL, KeyboardKey negative = KEY_NULL, int mouseAxis = -1, float mouseAxisScale = 1, int gamepadAxis = -1, float gamepadAxisScale = 1);

    ActionDef* GetAction(uint8_t id);

    float GetActionValue(uint8_t id) const;
    bool IsActionActive(uint8_t id) const;

protected:
    void OnInit() override;
    void OnUpdate() override;

protected:
    std::unordered_map<uint8_t, std::unique_ptr<ActionDef>>  Actions;
};