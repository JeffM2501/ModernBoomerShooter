#pragma once
#include "system.h"

#include <deque>
#include <string>
#include <vector>

class ConsoleRenderSystem : public System
{
public:
    DEFINE_SYSTM_NO_CONSTRUCTOR(ConsoleRenderSystem);
    ConsoleRenderSystem(World* world);
    ~ConsoleRenderSystem();

    bool WantKeyInput() const { return ConsoleState != State::Stowed; }

protected:
    void OnUpdate() override;

    void ProcessCommand();

    void OutputVarState(std::string_view name, const bool& value);
    void OutputVarState(std::string_view name, const float& value);
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
};