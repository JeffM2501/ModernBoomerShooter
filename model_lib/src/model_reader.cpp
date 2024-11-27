#include "model.h"
#include "raymath.h"

static ResolveModelTextureCallback TextureCallback = nullptr;

void SetModelTextureResolver(ResolveModelTextureCallback callback)
{
    TextureCallback = callback;
}

template<class T>
T ReadData(uint8_t* buffer, size_t& offset, size_t size)
{
    if (offset >= size)
        return 0;

    T value = *(T*)(buffer + offset);
    offset += sizeof(T);
    return value;
}

void ReadModel(Model& model, uint8_t* buffer, size_t size)
{
    model.transform = MatrixIdentity();
    size_t offset = 0;

    int version = ReadData<int>(buffer, offset, size);

    if (version != 1)
        return;

    model.meshCount = ReadData<int>(buffer, offset, size);
    model.materialCount = ReadData<int>(buffer, offset, size);

    model.meshes = (Mesh*)MemAlloc(sizeof(Mesh) * model.meshCount);
    model.meshMaterial = (int*)MemAlloc(sizeof(int) * model.meshCount);
    model.materials = (Material*)MemAlloc(sizeof(Material) * model.materialCount);

    for (int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
    {
        Mesh& mesh = model.meshes[meshIndex];

        mesh.vertexCount = ReadData<int>(buffer, offset, size);
        mesh.triangleCount = ReadData<int>(buffer, offset, size);
        model.meshMaterial[meshIndex] = ReadData<int>(buffer, offset, size);

        bool hasNormals = ReadData<int>(buffer, offset, size) != 0;
        bool hasColors = ReadData<int>(buffer, offset, size) != 0;
        bool hasIndecies = ReadData<int>(buffer, offset, size) != 0;

        mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
        memcpy(mesh.vertices, buffer + offset, mesh.vertexCount * 3 * sizeof(float));
        offset += mesh.vertexCount * 3 * sizeof(float);

        mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
        memcpy(mesh.texcoords, buffer + offset, mesh.vertexCount * 2 * sizeof(float));
        offset += mesh.vertexCount * 2 * sizeof(float);

        if (hasNormals)
        {
            mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
            memcpy(mesh.normals, buffer + offset, mesh.vertexCount * 3 * sizeof(float));
            offset += mesh.vertexCount * 3 * sizeof(float);
        }

        if (hasColors)
        {
            mesh.colors = (uint8_t*)MemAlloc(mesh.vertexCount * 4 * sizeof(uint8_t));
            memcpy(mesh.colors, buffer + offset, mesh.vertexCount * 4 * sizeof(uint8_t));
            offset += mesh.vertexCount * 4 * sizeof(uint8_t);
        }

        if (hasIndecies)
        {
            mesh.indices = (uint16_t*)MemAlloc(mesh.triangleCount * 3 * sizeof(uint16_t));
            memcpy(mesh.indices, buffer + offset, mesh.triangleCount * 3 * sizeof(uint16_t));
            offset += mesh.triangleCount * 3 * sizeof(uint16_t);
        }
    }

    for (int matIndex = 0; matIndex < model.materialCount; matIndex++)
    {
        Material& mat = model.materials[matIndex];
        mat = LoadMaterialDefault();

        mat.maps[MATERIAL_MAP_ALBEDO].color = GetColor(ReadData<int>(buffer, offset, size));

        int nameSize = ReadData<int>(buffer, offset, size);
        char* name = new char[nameSize + 1];
        name[nameSize] = '\0';
        memcpy(name, buffer + offset, nameSize);
        offset += nameSize;

        if (TextureCallback)
        {
            mat.maps[MATERIAL_MAP_ALBEDO].texture = TextureCallback(name);
        }
    }
}