#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"

#include <string>
#include <memory>

class AnimatedModelInstance;

class MobComponent : public Component
{
public:
    DEFINE_COMPONENT(MobComponent)

    void OnAddedToObject() override;

    void Draw();

protected:
    void OnCreate() override;

protected:
    std::shared_ptr<AnimatedModelInstance> Instance = nullptr;
};
