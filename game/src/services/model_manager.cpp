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

    if (!ModelGeometry.Groups.empty())
    {
        Vector3 temp = { 0 };
        for (auto& group : ModelGeometry.Groups)
        {
            for (auto& mesh : group.Meshes)
            {
                if (!BoundsValid)
                {
                    Bounds = GetMeshBoundingBox(mesh.Geometry);
                    BoundsValid = true;
                    break;
                }
                BoundingBox tempBounds = GetMeshBoundingBox(mesh.Geometry);

                temp.x = (Bounds.min.x < tempBounds.min.x) ? Bounds.min.x : tempBounds.min.x;
                temp.y = (Bounds.min.y < tempBounds.min.y) ? Bounds.min.y : tempBounds.min.y;
                temp.z = (Bounds.min.z < tempBounds.min.z) ? Bounds.min.z : tempBounds.min.z;
                Bounds.min = temp;

                temp.x = (Bounds.max.x > tempBounds.max.x) ? Bounds.max.x : tempBounds.max.x;
                temp.y = (Bounds.max.y > tempBounds.max.y) ? Bounds.max.y : tempBounds.max.y;
                temp.z = (Bounds.max.z > tempBounds.max.z) ? Bounds.max.z : tempBounds.max.z;
                Bounds.max = temp;
            }
        }
    }
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
    rlMultMatrixf(MatrixToFloatV(Geometry->OrientationTransform).v);

    Models::DrawAnimatableModel(Geometry->ModelGeometry, ModelIdentity, nullptr, &MaterialOverrides);
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
    if (geometry)
    {
        for (auto& group : geometry->ModelGeometry.Groups)
        {
            MaterialOverrides.push_back(LoadMaterialDefault());
            MaterialOverrides.back().shader = group.GroupMaterial.shader;

            for (int i = 0; i < 12; i++)
            {
                MaterialOverrides.back().maps[i].texture = group.GroupMaterial.maps[i].texture;
                MaterialOverrides.back().maps[i].color = group.GroupMaterial.maps[i].color;
            }
        }
    }
}

AnimatedModelInstance::AnimatedModelInstance(AnimatedModelRecord* geometry)
    : ModelInstance(geometry)
    , AnimatedModel(geometry)
{
    CurrentPose = Models::GetDefaultPose(Geometry->ModelGeometry);
}

void AnimatedModelInstance::Advance(float dt)
{
    if (CurrentAnimaton == nullptr)
        return;

    float animFrameTime = 1.0f / AnimationFPS;

    animFrameTime /= AnimationFPSMultiply;

    AnimationAccumulator += dt;
    while (AnimationAccumulator >= animFrameTime)
    {
        AnimationAccumulator -= animFrameTime;

        CurrentFrame++;
        if (CurrentFrame >= CurrentAnimaton->Frames.size())
            CurrentFrame = 0;
    }

    CurrentParam = AnimationAccumulator / animFrameTime;

    int lastFrame = CurrentFrame - 1;
    if (lastFrame < 0)
        lastFrame = int(CurrentAnimaton->Frames.size()) - 1;

    Models::InterpolatePose(Geometry->ModelGeometry, CurrentPose, CurrentAnimaton->Frames[lastFrame], CurrentAnimaton->Frames[CurrentFrame], CurrentParam);
}

void AnimatedModelInstance::Draw(class TransformComponent& transform)
{
    if (CurrentAnimaton == nullptr)
        return;

    rlPushMatrix();
    rlTranslatef(transform.Position.x, transform.Position.y, transform.Position.z);
    rlRotatef(transform.GetFacing(), 0, 0, 1);
    rlMultMatrixf(MatrixToFloatV(Geometry->OrientationTransform).v);

    Models::DrawAnimatableModel(Geometry->ModelGeometry, ModelIdentity, &CurrentPose, &MaterialOverrides);

    rlPopMatrix();
}

void AnimatedModelInstance::SetSequence(const std::string& name, int startFrame)
{
    auto itr = AnimatedModel->Animations.Sequences.find(name);
    if (itr == AnimatedModel->Animations.Sequences.end())
        return;


    CurrentAnimaton = &(itr->second);
    CurrentFrame = startFrame % CurrentAnimaton->Frames.size();
    AnimationAccumulator = 0;
}

namespace ModelManager
{
    static bool PreloadModels = false;

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

        modelRecord->ModelGeometry.Read(resource->DataBuffer, resource->DataSize);
        ResourceManager::ReleaseResource(resource);

        modelRecord->ModelGeometry.Upload();

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

        modelRecord->ModelGeometry.Read(resource->DataBuffer, resource->DataSize);
        ResourceManager::ReleaseResource(resource);

        if (!anim.empty())
        {
            resource = ResourceManager::OpenResource(anim);
            if (resource)
            {
                modelRecord->Animations.Read(resource->DataBuffer, resource->DataSize);
                ResourceManager::ReleaseResource(resource);
            }
        }

        modelRecord->ModelGeometry.Upload();

        AnimatedModelCache.insert_or_assign(nameRecord, modelRecord);
        return modelRecord.get();
    }

    void Init()
    {
        DefaultModel = std::make_shared<AnimatedModelRecord>();
        auto meshRecord = std::make_shared<ModelRecord>();

        Model tempModel = LoadModelFromMesh(GenMeshCube(0.5f, 0.5f, 0.5f));
        Models::LoadFromModel(DefaultModel->ModelGeometry, tempModel);
        DefaultModel->ModelGeometry.Groups[0].GroupMaterial.maps[MATERIAL_MAP_ALBEDO].color = MAGENTA;

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
        ModelCache.clear();
        AnimatedModelCache.clear();
    }
};
