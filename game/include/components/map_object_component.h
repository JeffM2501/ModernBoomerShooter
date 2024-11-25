#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"

#include <string>
#include <memory>

class ModelInstance;

class MapObjectComponent : public Component
{
public:
    DEFINE_COMPONENT(MapObjectComponent)

    MapObjectComponent(GameObject* owner, std::string_view modelName) : Component(owner), ModelName(modelName) {}

    void OnAddedToObject() override;

    std::string ModelName;
    std::shared_ptr<ModelInstance> Instance = nullptr;
    bool Solid = true;
};
