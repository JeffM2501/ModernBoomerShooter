#pragma once
#include "system.h"

#include <deque>
#include <string>
#include <vector>

class ConsoleRenderSystem : public System
{
public:
    DEFINE_SYSTEM(ConsoleRenderSystem);

    bool WantKeyInput() const { return ConsoleState != State::Stowed; }

protected:
    void OnSetup() override;
    void OnUpdate() override;

    void ProcessCommand();

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