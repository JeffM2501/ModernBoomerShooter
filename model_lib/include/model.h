#pragma once

#include "raylib.h"
#include <string>

void WriteModel(Model& model, std::string_view name);
void ReadModel(Model& model, std::string_view name);