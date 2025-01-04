#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "app.h"
#include "transform_tools.h"
#include "model.h"

#include "imgui.h"

#include <string>

void ProcessModel(const char* name);

namespace App
{
    bool Run = false;

    Models::AnimateableModel TheModel;

    AnimationState TheAnimations;

    const char* ModelName = nullptr;

    void InitGui();
    void ShowGui();
    void CleanupGui();
    void InitRender();
    void CleanupRender();
    void UpdateRender();
    void DrawRender();

    Models::AnimateableModel& GetModel()
    {
        return TheModel;
    }

    void RequestQuit()
    {
        Run = false;
    }
    
    void Init()
    {
        Run = true;
        SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(1280, 800, "Model Tool");
        SetTargetFPS(300);

        InitGui();
        InitRender();
    }

    void NewFrame()
    {
        UpdateRender();

        if (!ImGui::GetIO().WantCaptureKeyboard)
        {
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawRender();

        ShowGui();

        EndDrawing();
    }

    void Cleanup()
    {
        CleanupRender();
        CleanupGui();
        
        CloseWindow();
    }

    bool Quit()
    {
        return !Run || WindowShouldClose();
    }

    void LoadModel(const char* filename)
    {
        App::ModelName = GetFileNameWithoutExt(filename);

        Model tempModel = ::LoadModel(filename);

        Models::LoadFromModel(App::TheModel, tempModel);

        SetSeletedMesh(-1);
        SetSeletedBone(-1);

        TheAnimations.Frame = 0;
        TheAnimations.Sequence.clear();

        if (TheModel.RootBone != nullptr)
        {
            int count = 0;
            ModelAnimation* anims = ::LoadModelAnimations(filename, &count);
            Models::LoadFromAnimation(App::TheAnimations.Animations, App::TheModel, anims, count);
            MemFree(anims);
        }

        TheModel.AutoUnload = false;

        ModelUpdated();
    }

    void SaveStandardResource()
    {
        App::TheModel.Write(App::ModelName);
        if (!App::TheAnimations.Animations.Sequences.empty())
            App::TheAnimations.Animations.Write(App::ModelName);
    }

    AnimationState& GetAnimations()
    {
        return TheAnimations;
    }
}

void ProcessModel(const char* name)
{
    App::LoadModel(name);

    if (!App::TheModel.Groups.empty())
    {
        TransformTools::CenterMesh();
        TransformTools::RotateMesh(90, Vector3UnitX);
        TransformTools::FloorMesh();
        App::SaveStandardResource();
    }
}

int main(int argc, char* argv[])
{
    App::Init();

    std::string command;
    if (argc > 1)
        command = argv[1];

    if (command == "-generate")
    { 
        auto files = LoadDirectoryFilesEx("assets/models", ".glb", false);
        for (size_t i = 0; i < files.count; i++)
        {
            ProcessModel(files.paths[i]);
        }
    }
    else if (!command.empty())
    {
        ProcessModel(command.c_str());
    }
    
    while (!App::Quit())
    {
        App::NewFrame();
    }
    App::Cleanup();

    return 0;
}