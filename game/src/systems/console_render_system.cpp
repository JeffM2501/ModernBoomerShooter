#include "systems/console_render_system.h"
#include "services/game_time.h"
#include "services/global_vars.h"

#include "world.h"
#include "raylib.h"

#include <varargs.h>

static constexpr float AnimationTime = 0.5f;
static constexpr float ConsoleSizeParam = 0.75f;
static constexpr int ConsoleTextSize = 10;

inline const char* GetLogLevelName(int logLevel)
{
    switch (logLevel)
    {
    default:            return "All";
    case LOG_TRACE:     return "Trace";
    case LOG_DEBUG:     return "DEBUG";
    case LOG_INFO:      return "Info";
    case LOG_WARNING:   return "Warning";
    case LOG_ERROR:     return "ERROR";
    case LOG_FATAL:     return "FATAL";
    }
}

static ConsoleRenderSystem* LastConsole = nullptr;

ConsoleRenderSystem::ConsoleRenderSystem(World* world) : System(world)
{
    LastConsole = this;

    SetTraceLogCallback([](int logLevel, const char* text, va_list args)
        {
            if (!LastConsole)
                return;

            static char logText[2048] = { 0 };

            std::string log = GetLogLevelName(logLevel);
            vsprintf(logText, text, args);
            log += " ";
            log += logText;

            LastConsole->ConsoleOutput.push_front(log);
        });
}

ConsoleRenderSystem::~ConsoleRenderSystem()
{
	LastConsole = nullptr;
}

void ConsoleRenderSystem::OnUpdate()
{
	float animationTime = 0.25f;

	const char* text = nullptr;

	switch (ConsoleState)
	{
	case ConsoleRenderSystem::State::Stowed:
		if (IsKeyPressed(KEY_GRAVE))
		{
			ConsoleState = State::Deploying;
			Param = 0;
		}
		return;

	case ConsoleRenderSystem::State::Deploying:
		Param += GameTime::GetDeltaTime() / AnimationTime;
		text = TextFormat("Deploying %0.1f%%", Param * 100);
		if (Param >= 1)
		{
			ConsoleState = State::Deployed;
		}
		break;

	case ConsoleRenderSystem::State::Deployed:
		text = nullptr;
        if (IsKeyPressed(KEY_GRAVE))
        {
            ConsoleState = State::Stowing;
            Param = 1;
        }
		break;

	case ConsoleRenderSystem::State::Stowing:
		Param -= GameTime::GetDeltaTime() / AnimationTime;
        if (Param <= 0)
        {
            ConsoleState = State::Stowed;
			return;
        }
		text = TextFormat("Stowing %0.1f%%", Param * 100);
		break;

	default:
		break;
	}

	Rectangle consoleRect = { 0,0, float(GetScreenWidth()), float(GetScreenHeight()) * Param * ConsoleSizeParam };

	DrawRectangleRec(consoleRect, ColorAlpha(BLACK, 0.75f));
	DrawRectangleLinesEx(consoleRect, 3, ColorAlpha(RAYWHITE, 0.75f));
	DrawRectangleLinesEx(consoleRect, 1, ColorAlpha(DARKGRAY, 0.75f));
	BeginScissorMode(int(consoleRect.x), int(consoleRect.y), int (consoleRect.width), int(consoleRect.height));

    Rectangle textBoxRect = consoleRect;
    textBoxRect.y += textBoxRect.height - ConsoleTextSize-5;
    textBoxRect.height = ConsoleTextSize + 4;

    if (ConsoleState != State::Stowing)
    {
		while (int key = GetCharPressed())
		{
			CurrentConsoleInput += (char)key;
		}

		while (int key = GetKeyPressed())
		{
			if (key == KEY_BACKSPACE)
			{
				if (!CurrentConsoleInput.empty())
					CurrentConsoleInput.resize(CurrentConsoleInput.size() - 1);
			}
			else if (key == KEY_TAB)
			{
				if (!CurrentConsoleInput.empty())
				{

				}
			}
			else if (key == KEY_UP)
			{
				if (!ConsoleLog.empty() && CurrentHistoryLogItem != 0)
				{
					CurrentHistoryLogItem--;
					CurrentConsoleInput = ConsoleLog[CurrentHistoryLogItem];
				}
			}
			else if (key == KEY_DOWN)
			{
                if (!ConsoleLog.empty() && CurrentHistoryLogItem != ConsoleLog.size()-1)
                {
					CurrentHistoryLogItem++;
					CurrentConsoleInput = ConsoleLog[CurrentHistoryLogItem];
                }
			}
			else if (key == KEY_ESCAPE)
			{
				if (CurrentHistoryLogItem != ConsoleLog.size())
					CurrentConsoleInput.clear();
			}
			else if (key == KEY_ENTER || key == KEY_KP_ENTER)
			{
				if (!CurrentConsoleInput.empty())
					ProcessCommand();
			}
		}
		
		DrawRectangleLinesEx(textBoxRect, 1, ColorAlpha(YELLOW, 0.75f));
		DrawText(CurrentConsoleInput.c_str(), int(textBoxRect.x + 5), int(textBoxRect.y + 1), ConsoleTextSize, ColorAlpha(WHITE, 0.95f));

		if (int(GetTime() * 2) % 2 == 0)
		{
			Rectangle cursorRect = { float(MeasureText(CurrentConsoleInput.c_str(), ConsoleTextSize)) + textBoxRect.x + 7, textBoxRect.y + 2, 10.0f, ConsoleTextSize };
			DrawRectangleRec(cursorRect, ColorAlpha(WHITE, 0.75f));
		}	
    }

	float textY = (textBoxRect.y + textBoxRect.height) - (ConsoleTextSize*2 + 4);

	for (auto& item : ConsoleOutput)
	{
		if (textY < 0)
			break;

		DrawText(item.c_str(), int(textBoxRect.x+10), int(textY), ConsoleTextSize, ColorAlpha(WHITE, 0.75f));
		textY -= ConsoleTextSize;
	}

	if (text)
		DrawText(text, int(consoleRect.x + 2), int(consoleRect.y + 3), 10, ColorAlpha(WHITE, 0.5f));

	EndScissorMode();
}

void ConsoleRenderSystem::OutputVarState(std::string_view name, const bool& value)
{
	ConsoleOutput.push_front(TextFormat("%s = %s", name.data(), value ? "true" : "false"));
}

void ConsoleRenderSystem::OutputVarState(std::string_view name, const float& value) 
{
	ConsoleOutput.push_front(TextFormat("%s = %0.2f", name.data(), value));
}

void ConsoleRenderSystem::ProcessCommand()
{
    ConsoleOutput.push_front(CurrentConsoleInput);

	if (CurrentConsoleInput == "toggle_ghost")
	{
		GlobalVars::UseGhostMovement = !GlobalVars::UseGhostMovement;
		OutputVarState("UseGhostMovement", GlobalVars::UseGhostMovement);
	}

	if (ConsoleLog.empty() || ConsoleLog.back() != CurrentConsoleInput)
	{
		ConsoleLog.push_back(CurrentConsoleInput);
		CurrentHistoryLogItem = ConsoleLog.size();
	}

    CurrentConsoleInput.clear();
}
