#pragma once

#include <string_view>
#include "raylib.h"

namespace TextureManager
{
    void Init();
    void Cleanup();

    Texture2D GetTexture(std::string_view name);
    Texture2D GetTextureCubemap(std::string_view name);
    void UnloadAll();

    Shader GetShader(std::string_view name);

    size_t GetUsedVRAM();
};