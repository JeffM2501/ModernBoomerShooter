#pragma once

#include "raylib.h"

#include <string_view>
#include <stdint.h>
#include <functional>

void WriteModel(Model& model, std::string_view file);
void ReadModel(Model& model, uint8_t* buffer, size_t size, bool supportCPUAnimation = false);


void WriteModelAnimations(ModelAnimation* animations, size_t count, std::string_view file);
ModelAnimation* ReadModelAnimations(size_t& count, uint8_t* buffer, size_t size);

using ResolveModelTextureCallback = std::function<Texture(std::string_view)>;
void SetModelTextureResolver(ResolveModelTextureCallback callback);