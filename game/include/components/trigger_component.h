#pragma once

#include "component.h"
#include "game_object.h"

#include "raylib.h"
#include "map/map.h"


#include "systems/map_object_system.h"

class TriggerComponent : public Component
{
public:
    DEFINE_COMPONENT_WITH_SYSTEM(TriggerComponent, MapObjectSystem)

    TriggerComponent(GameObject* owner, const Rectangle& bounds) : Component(owner), Bounds(bounds) {}

    Rectangle Bounds = { 0 };

    static constexpr char TriggerEnter[] = "TriggerComponentEnter";
    static constexpr char TriggerExit[] = "TriggerComponentExit";
};