#pragma once

#include "raylib.h"
#include <string_view>
#include <stdint.h>
#include <functional>

void WriteModel(Model& model, std::string_view name);
void ReadModel(Model& model, uint8_t* buffer, size_t size);

using ResolveModelTextureCallback = std::function<Texture(std::string_view)>;
void SetModelTextureResolver(ResolveModelTextureCallback callback);