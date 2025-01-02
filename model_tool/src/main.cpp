#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "app.h"
#include "transform_tools.h"
#include "model.h"

#include "imgui.h"

#include "external/OpenFBX/ofbx.h"

#include <string>

void ProcessModel(const char* name);

namespace App
{
    bool Run = false;

    Model TheModel = { 0 };

    AnimationState TheAnimations;

    ModelAnimation* AnimPtr = nullptr;
    int AnimCount = 0;

    const char* ModelName = nullptr;

    void InitGui();
    void ShowGui();
    void CleanupGui();
    void InitRender();
    void CleanupRender();
    void UpdateRender();
    void DrawRender();

    Model& GetModel()
    {
        return TheModel;
    }

    void SetModel(Model& model)
    {
        TheModel = model;
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
        if (IsModelValid(App::TheModel))
            UnloadModel(App::TheModel);

        App::ModelName = GetFileNameWithoutExt(filename);

        App::TheModel = ::LoadModel(filename);

        SetSeletedMesh(-1);
        SetSeletedBone(-1);

        if (AnimPtr)
        {
            UnloadModelAnimations(AnimPtr, AnimCount);
            AnimPtr = nullptr;
            AnimCount = 0;
        }

        TheAnimations.Frame = -1;
        TheAnimations.Sequence = -1;
        TheAnimations.Animations.clear();

        if (TheModel.boneCount > 0)
        {
            AnimPtr = LoadModelAnimations(filename, &AnimCount);

            if (AnimPtr)
            {
                for (int i = 0; i < AnimCount; i++)
                    TheAnimations.Animations.push_back(AnimPtr + i);
            }
        }
    }

    void ImportFBX(const char* filename)
    {
        ofbx::IScene* scene = nullptr;

        ofbx::LoadFlags flags = ofbx::LoadFlags::NONE;

        int fileSize = 0;
        unsigned char* data = LoadFileData(filename, &fileSize);
        scene = ofbx::load((ofbx::u8*)data, fileSize, (ofbx::u16)flags);
        UnloadFileData(data);


        for (int i = 0; i < scene->getMeshCount(); i++)
        {
            const ofbx::Mesh& mesh = *scene->getMesh(i);
            const ofbx::GeometryData& geom = mesh.getGeometryData();

            const ofbx::Vec3Attributes positions = geom.getPositions();
            const ofbx::Vec3Attributes normals = geom.getNormals();
            const ofbx::Vec2Attributes uvs = geom.getUVs();

            for (int materaialGroup = 0; materaialGroup < geom.getPartitionCount(); materaialGroup++)
            {
                const ofbx::GeometryPartition& partition = geom.getPartition(materaialGroup);

                for (int polygonIndex = 0; polygonIndex < partition.polygon_count; polygonIndex++)
                {
                    const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygonIndex];

                    // todo read the geometry
                }
            }
        }
    }

    void SaveStandardResource()
    {
        WriteModel(App::TheModel, App::ModelName);
        if (App::AnimPtr != nullptr)
        {
            WriteModelAnimations(AnimPtr, AnimCount, ModelName);
        }
    }

    AnimationState& GetAnimations()
    {
        return TheAnimations;
    }
}

void ProcessModel(const char* name)
{
    App::LoadModel(name);

    if (IsModelValid(App::TheModel))
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