#include "services/model_manager.h"
#include "services/table_manager.h"
#include "services/resource_manager.h"
#include "components/transform_component.h"

#include "utilities/mesh_utils.h"
#include "utilities/string_utils.h"

#include "model.h"

#include "rlgl.h"
#include "raymath.h"

#include <unordered_map>
#include <string>

const Table* ModelManifestTable = nullptr;

BoundingBox ModelRecord::GetBounds()
{
    CheckBounds();
    return Bounds;
}

std::shared_ptr<ModelInstance> ModelRecord::GetModelInstance()
{
    ReferenceCount++;
    return std::make_shared<ModelInstance>(this);
}

void ModelRecord::ReleaseInstance()
{
    ReferenceCount--;

    if (ReferenceCount == 0)
    {
        // we could destroy this mesh
    }
}

void ModelRecord::CheckBounds()
{
    if (BoundsValid)
        return;
    Bounds = GetModelBoundingBox(Geometry);
}

ModelInstance::~ModelInstance()
{
    if (Geometry)
        Geometry->ReleaseInstance();
}

static Matrix ModelIdentity = MatrixScale(1,1,1);

void ModelInstance::Draw(TransformComponent& transform)
{
    rlPushMatrix();
    rlTranslatef(transform.Position.x, transform.Position.y, transform.Position.z);
    rlRotatef(transform.GetFacing(), 0, 0, 1);
    for (int mesh = 0; mesh < Geometry->Geometry.meshCount; mesh++)
    {
        DrawMesh(Geometry->Geometry.meshes[mesh], MaterialOverrides[Geometry->Geometry.meshMaterial[mesh]], ModelIdentity);
    }
    rlPopMatrix();
}

void ModelInstance::SetShader(Shader shader)
{
    for (auto& mat : MaterialOverrides)
        mat.shader = shader;
}

ModelInstance::ModelInstance(ModelRecord* geometry)
    :Geometry(geometry)
{
    for (size_t i = 0; i < geometry->Geometry.materialCount; i++)
    {
        MaterialOverrides.push_back(geometry->Geometry.materials[i]);
    }
}

AnimatedModelInstance::AnimatedModelInstance(AnimatedModelRecord* geometry)
    : ModelInstance(geometry)
    , AnimatedModel(geometry)
{

    for (size_t i = 0; i < geometry->Geometry.materialCount; i++)
    {
        MaterialOverrides.push_back(geometry->Geometry.materials[i]);
    }
}

void AnimatedModelInstance::Advance(float dt)
{
    if (CurrentSequence == nullptr)
        return;

    float animFrameTime = 1.0f / AnimationFPS;

    animFrameTime /= AnimationFPSMultiply;

    AnimationAccumulator += dt;
    while (AnimationAccumulator >= animFrameTime)
    {
        AnimationAccumulator -= animFrameTime;

        CurrentFrame++;
        if (CurrentFrame > CurrentSequence->frameCount)
            CurrentFrame = 0;
    }

    CurrentParam = AnimationAccumulator / animFrameTime;
    // TODO compute the interpolated frame data here (this can be in a thread)
}

void AnimatedModelInstance::Draw(class TransformComponent& transform)
{
    if (CurrentSequence == nullptr)
        return;

    // TODO, replace this with new animation drawing function when accepted into raylib
    UpdateModelAnimationBones(AnimatedModel->Geometry, *CurrentSequence, CurrentFrame);

    rlPushMatrix();
    rlTranslatef(transform.Position.x, transform.Position.y, transform.Position.z);
    rlRotatef(transform.GetFacing(), 0, 0, 1);
    rlMultMatrixf(MatrixToFloatV(AnimatedModel->Geometry.transform).v);

    for (int mesh = 0; mesh < Geometry->Geometry.meshCount; mesh++)
    {
        // TODO, replace with code that runs from a pose
        DrawMesh(Geometry->Geometry.meshes[mesh], MaterialOverrides[Geometry->Geometry.meshMaterial[mesh]], ModelIdentity);
    }
    rlPopMatrix();
}

void AnimatedModelInstance::SetSequence(const std::string& name, int startFrame)
{
    auto itr = AnimatedModel->AnimationSequences.find(name);
    if (itr == AnimatedModel->AnimationSequences.end())
        return;

    CurrentSequence = itr->second;
    CurrentFrame = startFrame % CurrentSequence->frameCount;
    AnimationAccumulator = 0;
}

namespace ModelManager
{
    static bool PreloadModels = true;

    std::shared_ptr<AnimatedModelRecord> DefaultModel;

    std::unordered_map<std::string, std::shared_ptr<ModelRecord>> ModelCache;
    std::unordered_map<std::string, std::shared_ptr<AnimatedModelRecord>> AnimatedModelCache;

    ModelRecord* FindModel(std::string_view name, std::string_view file)
    {
        std::string nameRecord(name);

        auto itr = ModelCache.find(nameRecord);
        if (itr != ModelCache.end())
            return itr->second.get();

        auto resource = ResourceManager::OpenResource(file);
        if (!resource)
            return DefaultModel.get();

        auto modelRecord = std::make_shared<ModelRecord>();
        ReadModel(modelRecord->Geometry, resource->DataBuffer, resource->DataSize);

        for (int mesh = 0; mesh < modelRecord->Geometry.meshCount; mesh++)
        {
            UploadMesh(&modelRecord->Geometry.meshes[mesh], true);
        }

        ResourceManager::ReleaseResource(resource);

        ModelCache.insert_or_assign(nameRecord, modelRecord);
        return modelRecord.get();
    }

    AnimatedModelRecord* FindAnimModel(std::string_view name, std::string_view record)
    {
        std::string nameRecord(name);

        auto itr = AnimatedModelCache.find(nameRecord);
        if (itr != AnimatedModelCache.end())
            return itr->second.get();

        auto parts = StringUtils::SplitString(record, ":");

        std::string file = parts[0];
        std::string anim;
        if (parts.size() > 1)
            anim = parts[1];

        auto resource = ResourceManager::OpenResource(file);
        if (!resource)
            return DefaultModel.get();

        auto modelRecord = std::make_shared<AnimatedModelRecord>();
        ReadModel(modelRecord->Geometry, resource->DataBuffer, resource->DataSize);

        for (int mesh = 0; mesh < modelRecord->Geometry.meshCount; mesh++)
        {
            UploadMesh(&modelRecord->Geometry.meshes[mesh], true);
        }

        ResourceManager::ReleaseResource(resource);

        if (!anim.empty())
        {
            resource = ResourceManager::OpenResource(anim);
            if (resource)
            {
                modelRecord->AnimationsPointer = ReadModelAnimations(modelRecord->Geometry, modelRecord->AnimationsCount, resource->DataBuffer, resource->DataSize);

                for (int i = 0; i < modelRecord->AnimationsCount; i++)
                {
                    modelRecord->AnimationSequences.insert_or_assign(std::string(modelRecord->AnimationsPointer[i].name), modelRecord->AnimationsPointer + i);
                }
            }
        }

        AnimatedModelCache.insert_or_assign(nameRecord, modelRecord);
        return modelRecord.get();
    }

    void Init()
    {
        DefaultModel = std::make_shared<AnimatedModelRecord>();
        auto meshRecord = std::make_shared<ModelRecord>();
        DefaultModel->Geometry = LoadModelFromMesh(GenMeshCube(0.5f, 0.5f, 0.5f));
        DefaultModel->Geometry.materials[0].maps[MATERIAL_MAP_ALBEDO].color = MAGENTA;

        ModelManifestTable = TableManager::GetTable(BootstrapTable)->GetFieldAsTable("model_manifest");

        if (ModelManifestTable && PreloadModels)
        {
            for (const auto& [key, file] : *ModelManifestTable)
            {
                FindModel(key, file);
            }
        }
    }

    void Cleanup()
    {
        UnloadAll();
    }

    std::shared_ptr<ModelInstance> GetModel(std::string_view name)
    {
        ModelRecord* model = nullptr;

        std::string nameRecord(name);

        auto itr = ModelCache.find(nameRecord);
        if (itr != ModelCache.end())
        {
            model = itr->second.get();
        }
        else if (!ModelManifestTable || !ModelManifestTable->HasField(nameRecord))
        {
            return nullptr;
        }
        else
        {
            model = FindModel(name, ModelManifestTable->GetField(nameRecord));
        }

        return std::make_shared<ModelInstance>(model);
    }

    std::shared_ptr<AnimatedModelInstance> GetAnimatedModel(std::string_view name)
    {
        AnimatedModelRecord* model = nullptr;

        std::string nameRecord(name);

        auto itr = AnimatedModelCache.find(nameRecord);
        if (itr != AnimatedModelCache.end())
        {
            model = itr->second.get();
        }
        else if (!ModelManifestTable || !ModelManifestTable->HasField(nameRecord))
        {
            return nullptr;
        }
        else
        {
            model = FindAnimModel(name, ModelManifestTable->GetField(nameRecord));
        }

        return std::make_shared<AnimatedModelInstance>(model);
    }

    void UnloadAll()
    {
        for (auto& [key, value] : ModelCache)
        {
            UnloadModel(value->Geometry);
        }

        ModelCache.clear();

        for (auto& [key, value] : AnimatedModelCache)
        {
            UnloadModel(value->Geometry);
            UnloadModelAnimations(value->AnimationsPointer, int(value->AnimationsCount));
        }

        AnimatedModelCache.clear();
    }
};
