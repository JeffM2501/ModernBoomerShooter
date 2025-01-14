#pragma once
#include "system.h"

#include <deque>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace ConsoleCommands
{
    static constexpr char Reload[] = "reload";

    static constexpr char ToggleCulling[] = "toggle_culling";
    static constexpr char ToggleGhost[] = "toggle_ghost";
    static constexpr char ToggleDebug[] = "toggle_debug";
    static constexpr char ToggleShowCoordinates[] = "show_coordinates";
    static constexpr char ToggleVSync[] = "toggle_vsync";

    static constexpr char SetConsoleFontSize[] = "set_console_font";
    static constexpr char SetFPSCap[] = "set_fps_cap";

    static constexpr char ListCommands[] = "list";
}

using CommandHandler = std::function<void(std::string_view, const std::vector<std::string>&)>;

class ConsoleRenderSystem : public System
{
public:
    DEFINE_SYSTM_NO_CONSTRUCTOR(ConsoleRenderSystem);
    ConsoleRenderSystem();
    ~ConsoleRenderSystem();

    bool WantKeyInput() const { return ConsoleState != State::Stowed; }

    void ProcessCommand(std::string_view command);

    void RegisterCommand(std::string_view command, CommandHandler handler);

protected:
    void OnUpdate() override;

    void OnSetup() override;

    void OutputVarState(std::string_view name, const bool& value);
    void OutputVarState(std::string_view name, const float& value);
    void OutputMessage(std::string_view name);
protected:
    enum class State
    {
        Stowed,
        Deploying,
        Deployed,
        Stowing,
    };

    State ConsoleState = State::Stowed;
    float Param = 0;

    std::deque<std::string> ConsoleOutput;

    std::vector<std::string> ConsoleLog;
    size_t CurrentHistoryLogItem = 0;

    std::string CurrentConsoleInput;
    std::map<std::string, CommandHandler> CommandHandlers;
};