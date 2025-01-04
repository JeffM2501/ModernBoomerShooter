#include "systems/console_render_system.h"
#include "services/game_time.h"
#include "services/global_vars.h"
#include "components/trigger_component.h"
#include "utilities/string_utils.h"
#include "utilities/debug_draw_utility.h"

#include "scene.h"
#include "raylib.h"

#include <string>
#include <stdarg.h>
#include <algorithm>

static constexpr float AnimationTime = 0.5f;
static constexpr float ConsoleSizeParam = 0.5f;
static int ConsoleTextSize = 10;

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

ConsoleRenderSystem::ConsoleRenderSystem() : System()
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

#if defined(_DEBUG)
			printf("%s : %s\n", log.c_str(), logText);
#endif
        });
}

ConsoleRenderSystem::~ConsoleRenderSystem()
{
	LastConsole = nullptr;
}

void ConsoleRenderSystem::RegisterCommand(std::string_view command, CommandHandler handler)
{
    CommandHandlers.insert_or_assign(std::string(command), handler);
}

void ConsoleRenderSystem::OnSetup()
{
	RegisterCommand(ConsoleCommands::ToggleGhost,
		[this](std::string_view command, const std::vector<std::string>& args)
		{  
			GlobalVars::UseGhostMovement = !GlobalVars::UseGhostMovement;
			OutputVarState("UseGhostMovement", GlobalVars::UseGhostMovement); 
		} );

	RegisterCommand(ConsoleCommands::ToggleCulling,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
            GlobalVars::UseVisCulling = !GlobalVars::UseVisCulling;
            OutputVarState("UseVisCulling", GlobalVars::UseVisCulling);
        });

    RegisterCommand(ConsoleCommands::ToggleVSync,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
			if (GlobalVars::UseVSync)
			{
				ClearWindowState(FLAG_VSYNC_HINT);
				GlobalVars::UseVSync = false;
			}
			else
			{
                SetWindowState(FLAG_VSYNC_HINT);
                GlobalVars::UseVSync = true;
			}
            OutputVarState("UseVsync", GlobalVars::UseVSync);
        });

	RegisterCommand(ConsoleCommands::Reload,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
            App::GetScene().ReloadMap();
            OutputMessage("Map Reloaded");
        });

	RegisterCommand(ConsoleCommands::ToggleDebug,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
			GlobalVars::ShowDebugDraw = !GlobalVars::ShowDebugDraw;
			OutputVarState("ShowDebugDraw", GlobalVars::ShowDebugDraw);
        });	
	
	RegisterCommand(ConsoleCommands::ToggleShowCoordinates,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
            GlobalVars::ShowCoordinates = !GlobalVars::ShowCoordinates;
            OutputVarState("ShowCoordinates", GlobalVars::ShowCoordinates);
        });

    RegisterCommand(ConsoleCommands::ListCommands,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
			OutputMessage("Available commands");
			for (auto& cmd : CommandHandlers)
			{
				OutputMessage(cmd.first);
			}
        });	
	
	RegisterCommand(ConsoleCommands::SetConsoleFontSize,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
			if (args.size() < 2)
				ConsoleTextSize = 10;
			else
				ConsoleTextSize = atoi(args[1].c_str());

			OutputMessage(TextFormat("Console Size = %d", ConsoleTextSize));
        });

    RegisterCommand(ConsoleCommands::SetFPSCap,
        [this](std::string_view command, const std::vector<std::string>& args)
        {
            if (args.size() < 2)
                GlobalVars::FPSCap = 300;
            else
				GlobalVars::FPSCap = atoi(args[1].c_str());

			SetTargetFPS(GlobalVars::FPSCap);

            OutputMessage(TextFormat("FPS Cap = %d", GlobalVars::FPSCap));
        });
}

void ConsoleRenderSystem::OnUpdate()
{
	float animationTime = 0.125f;

	// animation and state changes
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
		if (Param >= 1)
		{
			ConsoleState = State::Deployed;
		}
		break;

	case ConsoleRenderSystem::State::Deployed:
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
		break;

	default:
		break;
	}

	// draw overlay
	Rectangle consoleRect = { 0,0, float(GetScreenWidth()), float(GetScreenHeight()) * Param * ConsoleSizeParam };

	DrawRectangleRec(consoleRect, ColorAlpha(BLACK, 0.5f));
	DrawRectangleLinesEx(consoleRect, 3, ColorAlpha(RAYWHITE, 0.75f));
	DrawRectangleLinesEx(consoleRect, 1, ColorAlpha(DARKGRAY, 0.75f));
	BeginScissorMode(int(consoleRect.x), int(consoleRect.y), int (consoleRect.width), int(consoleRect.height));

    Rectangle textBoxRect = consoleRect;
    textBoxRect.y += textBoxRect.height - ConsoleTextSize-5;
    textBoxRect.height = float(ConsoleTextSize + 4);

	// handle keyboard input
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
					// TODO, auto complete?
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
				{
					ProcessCommand(CurrentConsoleInput);
					if (ConsoleLog.empty() || ConsoleLog.back() != CurrentConsoleInput)
					{
						ConsoleLog.push_back(CurrentConsoleInput);
						CurrentHistoryLogItem = ConsoleLog.size();
					}

					CurrentConsoleInput.clear();
				}
			}
		}
		
		// text box for user entry
		DrawRectangleLinesEx(textBoxRect, 1, ColorAlpha(YELLOW, 0.75f));
		DrawText(CurrentConsoleInput.c_str(), int(textBoxRect.x + 5), int(textBoxRect.y + 1), ConsoleTextSize, ColorAlpha(WHITE, 0.95f));

		// blinking cursor
		if (int(GetTime() * 2) % 2 == 0)
		{
			Rectangle cursorRect = { float(MeasureText(CurrentConsoleInput.c_str(), ConsoleTextSize)) + textBoxRect.x + 7, textBoxRect.y + 2, ConsoleTextSize * 0.5f, float(ConsoleTextSize)};
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

void ConsoleRenderSystem::OutputMessage(std::string_view name)
{
    ConsoleOutput.push_front(TextFormat("%s", name.data()));
}

void ConsoleRenderSystem::ProcessCommand(std::string_view command)
{
    ConsoleOutput.push_front(CurrentConsoleInput);

	auto args = StringUtils::SplitString(command, " ");

	if (args.empty())
		return;

    std::transform(args[0].begin(), args[0].end(), args[0].begin(), ::tolower);

	auto commandItr = CommandHandlers.find(args[0]);
	if (commandItr != CommandHandlers.end())
	{
		commandItr->second(args[0], args);
	}
	else
    {
        ConsoleOutput.push_front(TextFormat("Unknown command %s", command.data()));
    }
}
