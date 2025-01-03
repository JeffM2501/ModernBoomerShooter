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

Transform ReadTransform(uint8_t* buffer, size_t& offset, size_t size)
{
    Transform xform = { 0 };
    // translation
    xform.translation.x = ReadData<float>(buffer, offset, size);
    xform.translation.y = ReadData<float>(buffer, offset, size);
    xform.translation.z = ReadData<float>(buffer, offset, size);

    // rotation
    xform.rotation.x = ReadData<float>(buffer, offset, size);
    xform.rotation.y = ReadData<float>(buffer, offset, size);
    xform.rotation.z = ReadData<float>(buffer, offset, size);
    xform.rotation.w = ReadData<float>(buffer, offset, size);

    // scale
    xform.scale.x = ReadData<float>(buffer, offset, size);
    xform.scale.y = ReadData<float>(buffer, offset, size);
    xform.scale.z = ReadData<float>(buffer, offset, size);

    return xform;
}

void ReadMesh(Mesh& mesh, int& materialAssignment, uint8_t* buffer, size_t& offset, size_t size, bool useAnims)
{
    mesh.vertexCount = ReadData<int>(buffer, offset, size);
    mesh.triangleCount = ReadData<int>(buffer, offset, size);
    materialAssignment = ReadData<int>(buffer, offset, size);

    bool hasTextureCoords = ReadData<int>(buffer, offset, size) != 0;
    bool hasNormals = ReadData<int>(buffer, offset, size) != 0;
    bool hasColors = ReadData<int>(buffer, offset, size) != 0;
    bool hasIndecies = ReadData<int>(buffer, offset, size) != 0;

    bool hasBoneWeights = false;
    bool hasBoneIDs = false;
    if (useAnims)
    {
        hasBoneWeights = ReadData<int>(buffer, offset, size) != 0;
        hasBoneIDs = ReadData<int>(buffer, offset, size) != 0;
    }

    int bufferSize = 0;

    bufferSize = mesh.vertexCount * 3 * sizeof(float);
    mesh.vertices = (float*)MemAlloc(bufferSize);
    memcpy(mesh.vertices, buffer + offset, bufferSize);
    offset += bufferSize;

    bufferSize = mesh.vertexCount * 2 * sizeof(float);
    mesh.texcoords = (float*)MemAlloc(bufferSize);
    if (hasTextureCoords)
    {
        memcpy(mesh.texcoords, buffer + offset, bufferSize);
        offset += bufferSize;
    }

    if (hasNormals)
    {
        bufferSize = mesh.vertexCount * 3 * sizeof(float);
        mesh.normals = (float*)MemAlloc(bufferSize);
        memcpy(mesh.normals, buffer + offset, bufferSize);
        offset += bufferSize;
    }

    if (hasColors)
    {
        bufferSize = mesh.vertexCount * 4 * sizeof(uint8_t);
        mesh.colors = (uint8_t*)MemAlloc(bufferSize);
        memcpy(mesh.colors, buffer + offset, bufferSize);
        offset += bufferSize;
    }

    if (hasIndecies)
    {
        bufferSize = mesh.triangleCount * 3 * sizeof(uint16_t);
        mesh.indices = (uint16_t*)MemAlloc(bufferSize);
        memcpy(mesh.indices, buffer + offset, bufferSize);
        offset += bufferSize;
    }

    if (hasBoneWeights)
    {
        bufferSize = mesh.vertexCount * 4 * sizeof(float);
        mesh.boneWeights = (float*)MemAlloc(bufferSize);
        memcpy(mesh.boneWeights, buffer + offset, bufferSize);
        offset += bufferSize;
    }

    if (hasBoneIDs)
    {
        bufferSize = mesh.vertexCount * 4 * sizeof(unsigned char);
        mesh.boneIds = (unsigned char*)MemAlloc(bufferSize);
        memcpy(mesh.boneIds, buffer + offset, bufferSize);
        offset += bufferSize;
    }
}

void ReadModel(Model& model, uint8_t* buffer, size_t size, bool supportCPUAnimation)
{
    model.transform = MatrixIdentity();
    size_t offset = 0;

    int version = ReadData<int>(buffer, offset, size);

    if (version < 1 || version > 3)
        return;

    bool useAnims = version >= 2;
    bool hasTransform = version >= 3;

    model.meshCount = ReadData<int>(buffer, offset, size);
    model.materialCount = ReadData<int>(buffer, offset, size);

    model.meshes = (Mesh*)MemAlloc(sizeof(Mesh) * model.meshCount);
    model.meshMaterial = (int*)MemAlloc(sizeof(int) * model.meshCount);
    model.materials = (Material*)MemAlloc(sizeof(Material) * model.materialCount);

    for (int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
    {
        Mesh& mesh = model.meshes[meshIndex];
        ReadMesh(mesh, model.meshMaterial[meshIndex], buffer, offset, size, useAnims);
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

    if (useAnims)
    {
        model.boneCount = ReadData<int>(buffer, offset, size);
        if (model.boneCount > 0)
        {
            model.bones = (BoneInfo*)MemAlloc(model.boneCount * sizeof(BoneInfo));
            model.bindPose = (Transform*)MemAlloc(model.boneCount * sizeof(Transform));

            for (int i = 0; i < model.boneCount; i++)
            {
                // bone info
                model.bones[i].parent = ReadData<int>(buffer, offset, size);
                memcpy(model.bones[i].name, buffer + offset, 32);
                offset += 32;

                // bind pose transform
                model.bindPose[i] = ReadTransform(buffer, offset, size);
            }

            for (int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
            {
                auto& mesh = model.meshes[meshIndex];
                mesh.boneCount = model.boneCount;
                mesh.boneMatrices = (Matrix*)MemAlloc(sizeof(Matrix) * mesh.boneCount);
                for (int i = 0; i < model.boneCount; i++)
                {
                    mesh.boneMatrices[i] = MatrixIdentity();
                }

                if (supportCPUAnimation)
                {
                    // Animated vertex data
                    model.meshes[meshIndex].animVertices = (float*)MemAlloc(model.meshes[meshIndex].vertexCount * 3 * sizeof(float));
                    memcpy(model.meshes[meshIndex].animVertices, model.meshes[meshIndex].vertices, model.meshes[meshIndex].vertexCount * 3 * sizeof(float));

                    if (model.meshes[meshIndex].normals != nullptr)
                    {
                        model.meshes[meshIndex].animNormals = (float*)MemAlloc(model.meshes[meshIndex].vertexCount * 3 * sizeof(float));
                        memcpy(model.meshes[meshIndex].animNormals, model.meshes[meshIndex].normals, model.meshes[meshIndex].vertexCount * 3 * sizeof(float));
                    }
                }
            }
        }
    }

    if (hasTransform)
    {
        Transform temp = ReadTransform(buffer, offset, size);

        Matrix matScale = MatrixScale(temp.scale.x, temp.scale.y, temp.scale.z);
        Vector3 axis = Vector3Zeros;
        float angle = 0;
        QuaternionToAxisAngle(temp.rotation, &axis, &angle);
        Matrix matRotation = MatrixRotate(axis, angle);
        Matrix matTranslation = MatrixTranslate(temp.translation.x, temp.translation.y, temp.translation.z);
        model.transform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    }
}

ModelAnimation* ReadModelAnimations(const Model& model, size_t& count, uint8_t* buffer, size_t size)
{
    size_t offset = 0;

    int version = ReadData<int>(buffer, offset, size);

    if (version != 1)
        return nullptr;

    count = size_t(ReadData<uint32_t>(buffer, offset, size));

    ModelAnimation* animations = (ModelAnimation*)MemAlloc(sizeof(ModelAnimation) * (int)count);

    for (size_t animIndex = 0; animIndex < count; animIndex++)
    {
        ModelAnimation& anim = animations[animIndex];

        memcpy(anim.name, buffer + offset, 32);
        offset += 32;

        anim.boneCount = ReadData<int>(buffer, offset, size);
        anim.frameCount = ReadData<int>(buffer, offset, size);

        anim.framePoses = (Transform**)MemAlloc(sizeof(Transform*) *  anim.frameCount);

        anim.bones = (BoneInfo*)MemAlloc(sizeof(BoneInfo) * model.boneCount);

        memcpy(anim.bones, model.bones, sizeof(BoneInfo) * model.boneCount);

        for (size_t frameIndex = 0; frameIndex < anim.frameCount; frameIndex++)
        {
            anim.framePoses[frameIndex] = (Transform*)MemAlloc(sizeof(Transform) * anim.boneCount);

            for (size_t boneIndex = 0; boneIndex < anim.boneCount; boneIndex++)
            {
                anim.framePoses[frameIndex][boneIndex] = ReadTransform(buffer, offset, size);
            }
        }
    }

    return animations;
}

namespace Models
{
    void AnimationSet::Read(uint8_t* buffer, size_t size)
    {
        Sequences.clear();

        size_t offset = 0;

        int version = ReadData<int>(buffer, offset, size);

        if (version != 1)
            return;

        size_t count = size_t(ReadData<uint32_t>(buffer, offset, size));

        for (size_t animIndex = 0; animIndex < count; animIndex++)
        {
            char name[32] = { 0 };
            memcpy(name, buffer + offset, 32);
            offset += 32;

            auto& anim = Sequences.try_emplace(std::string(name)).first->second;

            int boneCount = ReadData<int>(buffer, offset, size);
            int frameCount = ReadData<int>(buffer, offset, size);

            for (size_t frameIndex = 0; frameIndex < frameCount; frameIndex++)
            {
                auto& frame = anim.Frames.emplace_back();

                for (size_t boneIndex = 0; boneIndex < boneCount; boneIndex++)
                {
                    frame.GlobalTransforms.push_back(ReadTransform(buffer, offset, size));
                }
            }
        }
    }

    void AnimateableModel::Read(uint8_t* buffer, size_t size)
    {
        RootTransform = MatrixIdentity();

        size_t offset = 0;

        int version = ReadData<int>(buffer, offset, size);

        if (version < 1 || version > 3)
            return;

        bool useAnims = version >= 2;
        bool hasTransform = version >= 3;

        int meshCount = ReadData<int>(buffer, offset, size);
        int materialCount = ReadData<int>(buffer, offset, size);
        Groups.clear();

        Groups.resize(materialCount);

        for (int meshIndex = 0; meshIndex < meshCount; meshIndex++)
        {
            Mesh tempMesh = { 0 };
            int assignment = 0;
            ReadMesh(tempMesh, assignment, buffer, offset, size, useAnims);

            Groups[assignment].Meshes.emplace_back();
            Groups[assignment].Meshes.back().Geometry = tempMesh;
        }

        for (int matIndex = 0; matIndex < materialCount; matIndex++)
        {
            auto& mat = Groups[matIndex];
            mat.GroupMaterial = LoadMaterialDefault();

            mat.GroupMaterial.maps[MATERIAL_MAP_ALBEDO].color = GetColor(ReadData<int>(buffer, offset, size));

            int nameSize = ReadData<int>(buffer, offset, size);
            char* name = new char[nameSize + 1];
            name[nameSize] = '\0';
            memcpy(name, buffer + offset, nameSize);
            offset += nameSize;

            if (TextureCallback)
            {
                mat.GroupMaterial.maps[MATERIAL_MAP_ALBEDO].texture = TextureCallback(name);
            }
        }

        if (useAnims)
        {
            int boneCount = ReadData<int>(buffer, offset, size);
            if (boneCount > 0)
            {
                for (int i = 0; i < boneCount; i++)
                {
                    auto& bone = Bones.emplace_back();
                    // bone info
                    bone.ParentBoneId = ReadData<int>(buffer, offset, size);
                    char name[32] = { 0 };
                    memcpy(name, buffer + offset, 32);
                    offset += 32;

                    bone.Name = name;
                    bone.DefaultGlobalTransform = ReadTransform(buffer, offset, size);
                }

                // build the bone tree
                for (auto& bone : Bones)
                {
                    if (bone.ParentBoneId == -1)
                        RootBone = &bone;
                    else
                        Bones[bone.ParentBoneId].Children.push_back(&bone);
                }
            }
        }

        if (hasTransform)
        {
            Transform temp = ReadTransform(buffer, offset, size);

            Matrix matScale = MatrixScale(temp.scale.x, temp.scale.y, temp.scale.z);
            Vector3 axis = Vector3Zeros;
            float angle = 0;
            QuaternionToAxisAngle(temp.rotation, &axis, &angle);
            Matrix matRotation = MatrixRotate(axis, angle);
            Matrix matTranslation = MatrixTranslate(temp.translation.x, temp.translation.y, temp.translation.z);
            RootTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
        }
    }
}