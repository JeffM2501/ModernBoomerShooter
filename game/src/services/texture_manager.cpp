#include "services/texture_manager.h"
#include "services/resource_manager.h"
#include "services/table_manager.h"
#include "model.h"

#include <unordered_map>

namespace TextureManager
{
    size_t UsedVRam = 0;

    struct TextureRecord
    {
        size_t Hash = 0;
        ::Texture2D Texture = { 0 };
        size_t ImageSize = 0;
    };

    static std::unordered_map<size_t, TextureRecord> LoadedTextures;

    static bool PreloadShaders = true;
    static std::unordered_map<size_t, Shader> LoadedShaders;

    static std::hash<std::string_view> StringHasher;

    static TextureRecord DefaultTexture;

    static const Table* ShaderTable = nullptr;

    void LoadTextureRecord(TextureRecord& record, Image& image)
    {
        record.Texture = LoadTextureFromImage(image);
        GenTextureMipmaps(&record.Texture);
        SetTextureFilter(record.Texture, TEXTURE_FILTER_ANISOTROPIC_16X);

        record.ImageSize = image.width * image.height;
        if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8)
            record.ImageSize *= 3;
        if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            record.ImageSize *= 4;

        record.ImageSize += record.ImageSize / 3;
  
        UsedVRam += record.ImageSize;
    }

    Shader FindShader(std::string_view key, std::string_view vertex, std::string_view fragment)
    {
        size_t hash = StringHasher(key);

        auto itr = LoadedShaders.find(hash);
        if (itr != LoadedShaders.end())
            return itr->second;

        auto vertexResource = ResourceManager::OpenResource(vertex, true);
        auto fragmentResource = ResourceManager::OpenResource(fragment, true);

        const char* vertexText = nullptr;
        if (vertexResource)
            vertexText = (char*)(vertexResource->DataBuffer);

        const char* fragmentText = nullptr;
        if (fragmentResource)
            fragmentText = (char*)(fragmentResource->DataBuffer);

        Shader shader = LoadShaderFromMemory(vertexText, fragmentText);

        ResourceManager::ReleaseResource(vertexResource);
        ResourceManager::ReleaseResource(fragmentResource);

        LoadedShaders.insert_or_assign(hash, shader);
        return shader;
    }

    void Init()
    {
        auto defaultImage = GenImageChecked(128, 128, 8, 8, DARKGRAY, GRAY);
        ImageDrawRectangle(&defaultImage, 32, 8, 64, 10, ColorAlpha(RED, 0.5f));
        ImageDrawRectangle(&defaultImage, 64-8, 32, 16, 64, ColorAlpha(GREEN, 0.5f));
        ImageDrawRectangle(&defaultImage, 0, 128-8, 8, 8, ColorAlpha(BLUE, 0.5f));
        ImageDrawRectangle(&defaultImage, 128-8, 128 - 8, 8, 8, ColorAlpha(YELLOW, 0.5f));

        LoadTextureRecord(DefaultTexture, defaultImage);
        UnloadImage(defaultImage);

        SetModelTextureResolver(GetTexture);

        auto* bootstrapTable = TableManager::GetTable(BootstrapTable);
        
        ShaderTable = bootstrapTable->GetFieldAsTable("shader_manifest");

        if (ShaderTable && PreloadShaders)
        {
            for (const auto& [key, files] : *ShaderTable)
            { 
                if (key.empty())
                    continue;

                size_t hash = StringHasher(key);

                auto shaderFiles = ShaderTable->SplitField(key, ":");
                if (shaderFiles.empty())
                    continue;
                
                std::string fragShader = shaderFiles[0];
                std::string vertShader;
                if (shaderFiles.size() > 1)
                    vertShader = shaderFiles[1];

                FindShader(key, vertShader, fragShader);
            }
        }
    }

    Shader GetShader(std::string_view name)
    {
        size_t hash = StringHasher(name);
        
        auto itr = LoadedShaders.find(hash);
        if (itr != LoadedShaders.end())
            return itr->second;

        std::string shaderName(name);

        if (!ShaderTable || !ShaderTable->HasField(shaderName))
            return LoadShader(nullptr, nullptr);

        auto shaderFiles = ShaderTable->SplitField(shaderName, ";");
        std::string vertShader;
        std::string fragShader;

        if (shaderFiles.size() > 0)
            fragShader = shaderFiles[0];
        if (shaderFiles.size() > 1)
            vertShader = shaderFiles[1];

        return FindShader(shaderName, vertShader, fragShader);
    }

    void Cleanup()
    {
        UnloadAll();

        UnloadTexture(DefaultTexture.Texture);

        UsedVRam -= DefaultTexture.ImageSize;

        DefaultTexture.Texture.id = 0;
        DefaultTexture.ImageSize = 0;
    }

    Texture2D GetTexture(std::string_view name)
    {
        size_t hash = StringHasher(name);
        auto itr = LoadedTextures.find(hash);
        if (itr != LoadedTextures.end())
            return itr->second.Texture;

        auto resource = ResourceManager::OpenResource(name);
        if (!resource)
        {
            return DefaultTexture.Texture;
        }

        itr = LoadedTextures.try_emplace(hash).first;

        Image img = LoadImageFromMemory(GetFileExtension(name.data()), resource->DataBuffer, int(resource->DataSize));
        LoadTextureRecord(itr->second, img);
        UnloadImage(img);
        ResourceManager::ReleaseResource(resource);

        return itr->second.Texture;
    }

    static bool GenerateCubeMapMipMaps = false;

    Texture2D GetTextureCubemap(std::string_view name)
    {
        size_t hash = StringHasher(name);
        auto itr = LoadedTextures.find(hash);
        if (itr != LoadedTextures.end())
            return itr->second.Texture;

        auto resource = ResourceManager::OpenResource(name);
        if (!resource)
            return DefaultTexture.Texture;

        Image image = LoadImageFromMemory(GetFileExtension(name.data()), resource->DataBuffer, int(resource->DataSize));
        ResourceManager::ReleaseResource(resource);
        if (IsImageValid(image))
        {
            itr = LoadedTextures.try_emplace(hash).first;

            TextureRecord& record = itr->second;

            record.ImageSize = image.width * image.height;
            if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8)
                record.ImageSize *= 3;
            if (image.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
                record.ImageSize *= 4;

            record.Texture = LoadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT);    // CUBEMAP_LAYOUT_PANORAMA

            if (GenerateCubeMapMipMaps)
            {
                GenTextureMipmaps(&record.Texture);
                SetTextureFilter(record.Texture, TEXTURE_FILTER_ANISOTROPIC_16X);
                record.ImageSize += record.ImageSize / 3;
            }

            UsedVRam += record.ImageSize;
            UnloadImage(image);

            return record.Texture;
        }
        else
        {
            return DefaultTexture.Texture;
        }
    }

    void UnloadAll()
    {
        for (auto& [hash, textureRecord] : LoadedTextures)
        {
            UsedVRam -= textureRecord.ImageSize;
            UnloadTexture(textureRecord.Texture);
        }

        LoadedTextures.clear();

        for (auto& [hash, shader] : LoadedShaders)
        {
            UnloadShader(shader);
        }
        LoadedShaders.clear();
    }

    size_t GetUsedVRAM()
    {
        return UsedVRam;
    }
};