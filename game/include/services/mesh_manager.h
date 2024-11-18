#pragma once

#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <string_view>

class MeshRecord
{
public:
    Mesh Geometry;
    Material BaseMaterial;

protected:
    size_t ReferenceCount = 0;

protected:
    friend class MeshInstance;
    std::shared_ptr<MeshInstance> GetMeshInstance();

    void ReleaseInstance();
};

class MeshInstance
{
public:
    ~MeshInstance();
public:
    MeshRecord* Mesh;
    void Draw(class TransformComponent& transform);

    MeshInstance(MeshRecord* mesh);

};

namespace MeshManager
{
    void Init();
    void Cleanup();

    std::shared_ptr<MeshInstance> GetMesh(std::string_view name);
    void UnloadAll();
};