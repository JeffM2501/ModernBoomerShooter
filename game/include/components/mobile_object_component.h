#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"

#include <string>
#include <memory>

class MobComponent : public Component
{
public:
    DEFINE_COMPONENT(MobComponent)

    void OnAddedToObject() override;

    void Draw();
};
