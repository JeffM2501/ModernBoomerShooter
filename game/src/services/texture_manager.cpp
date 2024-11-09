#include "services/texture_manager.h"
#include "services/resource_manager.h"

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

    std::unordered_map<size_t, TextureRecord> LoadedTextures;

    static std::hash<std::string_view> StringHasher;

    TextureRecord DefaultTexture;

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

    void Init()
    {
        auto defaultImage = GenImageChecked(128, 128, 8, 8, DARKGRAY, GRAY);
        ImageDrawRectangle(&defaultImage, 32, 8, 64, 10, ColorAlpha(RED, 0.5f));
        ImageDrawRectangle(&defaultImage, 64-8, 32, 16, 64, ColorAlpha(GREEN, 0.5f));
        ImageDrawRectangle(&defaultImage, 0, 128-8, 8, 8, ColorAlpha(BLUE, 0.5f));
        ImageDrawRectangle(&defaultImage, 128-8, 128 - 8, 8, 8, ColorAlpha(YELLOW, 0.5f));

        LoadTextureRecord(DefaultTexture, defaultImage);
        UnloadImage(defaultImage);
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

    void UnloadAll()
    {
        for (auto& [hash, textureRecord] : LoadedTextures)
        {
            UsedVRam -= textureRecord.ImageSize;
            UnloadTexture(textureRecord.Texture);
        }

        LoadedTextures.clear();
    }

    size_t GetUsedVRAM()
    {
        return UsedVRam;
    }
};