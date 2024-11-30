#pragma once

#include "component.h"
#include "game_object.h"

#include "raylib.h"
#include "map/map.h"

class PlayerInfoComponent : public Component
{
public:
    DEFINE_COMPONENT(PlayerInfoComponent)

    uint32_t PlayerId = uint32_t(-1);

};