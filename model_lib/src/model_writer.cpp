#include "model.h"

#include <cstring>
#include <string>
#include <stdio.h>
#include <span>

template<class T>
static void Write(FILE* fp, T value)
{
    fwrite(&value, sizeof(T), 1, fp);
}

static void WriteTransform(FILE* out, Transform& xform)
{
    // translation
    Write(out, xform.translation.x);
    Write(out, xform.translation.y);
    Write(out, xform.translation.z);

    // rotation
    Write(out, xform.rotation.x);
    Write(out, xform.rotation.y);
    Write(out, xform.rotation.z);
    Write(out, xform.rotation.w);

    // scale
    Write(out, xform.scale.x);
    Write(out, xform.scale.y);
    Write(out, xform.scale.z);
}

using bytes = std::span<const std::byte>;

void WriteModel(Model& model, std::string_view file)
{
    std::string outputPath = "resources/models/";
    outputPath += file.data();
    outputPath += ".mesh";

    FILE* out = fopen(outputPath.c_str(), "wb");

    if (!out)
        return;

    int temp = 1;

    Write(out, 2);
    Write(out, model.meshCount);
    Write(out, model.materialCount);

    for (int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
    {
        Mesh& mesh = model.meshes[meshIndex];
        Write(out, mesh.vertexCount);
        Write(out, mesh.triangleCount);
        Write(out, model.meshMaterial[meshIndex]);

        Write(out, mesh.texcoords ? int(1) : int(0));
        Write(out, mesh.normals ? int(1) : int(0));
        Write(out, mesh.colors ? int(1) : int(0));
        Write(out, mesh.indices ? int(1) : int(0));
        Write(out, mesh.boneWeights ? int(1) : int(0));
        Write(out, mesh.boneIds ? int(1) : int(0));

        fwrite(mesh.vertices, sizeof(float) * mesh.vertexCount * 3, 1, out);
        if (mesh.texcoords)
            fwrite(mesh.texcoords, sizeof(float) * mesh.vertexCount * 2, 1, out);

        if (mesh.normals)
            fwrite(mesh.normals, sizeof(float) * mesh.vertexCount * 3, 1, out);

        if (mesh.colors)
            fwrite(mesh.colors, sizeof(char) * mesh.vertexCount * 4, 1, out);

        if (mesh.indices)
            fwrite(mesh.indices, sizeof(unsigned short) * mesh.triangleCount * 3, 1, out);
        
        if (mesh.boneWeights)
            fwrite(mesh.boneWeights, sizeof(float) * mesh.vertexCount * 4, 1, out);

        if (mesh.boneIds)
            fwrite(mesh.boneIds, sizeof(unsigned char) * mesh.vertexCount * 4, 1, out);
    }

    std::hash<std::string_view> hasher;

    for (int matIndex = 0; matIndex < model.materialCount; matIndex++)
    { 
        Material& mat = model.materials[matIndex];

        Image texture = LoadImageFromTexture(mat.maps[MATERIAL_MAP_ALBEDO].texture);

        int bpp = texture.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 ? 4 : 3;

        std::string_view view((const char*)texture.data, texture.width * texture.height * bpp);
        size_t hash = hasher(view);

        const char* textureName = TextFormat("textures/models/%d.png", hash);
        ExportImage(texture, textureName);

        Write(out, ColorToInt(mat.maps[MATERIAL_MAP_ALBEDO].color));
        Write(out, int(strlen(textureName)));
        fwrite(textureName, strlen(textureName), 1, out);
    }

    Write(out, model.boneCount);
    if (model.boneCount > 0)
    {
        for (int i = 0; i < model.boneCount; i++)
        {
            // bone info
            Write(out, model.bones[i].parent);
            fwrite(model.bones[i].name, 32, 1, out);

            // bind pose transform
            WriteTransform(out, model.bindPose[i]);
        }
    }

    fclose(out);
}

void WriteModelAnimations(ModelAnimation* animations, size_t count, std::string_view file)
{
    if (!animations || count == 0)
        return;

    std::string outputPath = "resources/models/";
    outputPath += file.data();
    outputPath += ".anim";

    FILE* out = fopen(outputPath.c_str(), "wb");

    if (!out)
        return;

    int temp = 1;

    Write(out, 1);
    Write(out, uint32_t(count));

    for (size_t animIndex = 0; animIndex < count; animIndex++)
    {
        ModelAnimation& anim = animations[animIndex];

        fwrite(anim.name, 32, 1, out);
        Write(out, anim.boneCount);
        Write(out, anim.frameCount);

        for (size_t frameIndex = 0; frameIndex < anim.frameCount; frameIndex++)
        {
            for (size_t boneIndex = 0; boneIndex < anim.boneCount; boneIndex++)
            {
                WriteTransform(out, anim.framePoses[frameIndex][boneIndex]);
            }
        }
    }
    fclose(out);
}