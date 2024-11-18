#include "services/mesh_manager.h"
#include "services/table_manager.h"
#include "services/resource_manager.h"

#include "utilities/mesh_utils.h"

#include <unordered_map>
#include <string>

const Table* MeshManifestTable = nullptr;

std::shared_ptr<MeshInstance> MeshRecord::GetMeshInstance()
{
    ReferenceCount++;
    return std::make_shared<MeshInstance>(this);
}

void MeshRecord::ReleaseInstance()
{
    ReferenceCount--;

    if (ReferenceCount == 0)
    {
        // we could destroy this mesh
    }
}

MeshInstance::~MeshInstance()
{
    if (Mesh)
        Mesh->ReleaseInstance();
}


void MeshInstance::Draw(class TransformComponent& transform)
{

}

MeshInstance::MeshInstance(MeshRecord* mesh)
    :Mesh(mesh)
{
}

namespace MeshManager
{
    static bool PreloadMeshes = true;

    std::shared_ptr<MeshRecord> DefaultMesh;

    std::unordered_map<std::string, std::shared_ptr<MeshRecord>> MesheCache;

    MeshRecord* FindMesh(std::string_view name)
    {
        std::string nameRecord(name);

        auto itr = MesheCache.find(nameRecord);
        if (itr != MesheCache.end())
            return itr->second.get();

        auto meshRecord = std::make_shared<MeshRecord>();
        DefaultMesh->Geometry = GenMeshCube(0.5f, 0.5f, 0.5f);
        DefaultMesh->BaseMaterial = LoadMaterialDefault();
        DefaultMesh->BaseMaterial.maps[MATERIAL_MAP_ALBEDO].color = MAGENTA;
        return nullptr;

    }

    void Init()
    {
        DefaultMesh = std::make_shared<MeshRecord>();
        DefaultMesh->Geometry = GenMeshCube(0.5f, 0.5f, 0.5f);
        DefaultMesh->BaseMaterial = LoadMaterialDefault();
        DefaultMesh->BaseMaterial.maps[MATERIAL_MAP_ALBEDO].color = MAGENTA;

        MeshManifestTable = TableManager::GetTable(BootstrapTable)->GetFieldAsTable("model_manifest");

        if (MeshManifestTable && PreloadMeshes)
        {
            for (const auto& [key, file] : *MeshManifestTable)
            {

            }
        }
    }

    void Cleanup()
    {

    }

    std::shared_ptr<MeshInstance> GetMesh(std::string_view name)
    {
        return nullptr;
    }

    void UnloadAll()
    {

    }
};
