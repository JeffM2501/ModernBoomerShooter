#pragma once

#include <string_view>
#include "raylib.h"

namespace TextureManager
{
    void Init();
    void Cleanup();

    Texture2D GetTexture(std::string_view name);
    void UnloadAll();

    size_t GetUsedVRAM();
};