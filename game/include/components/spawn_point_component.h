#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"

#include "systems/player_management_system.h"

enum class SpawnType
{
    Player,
};

class SpawnPointComponent : public Component
{
public:
    DEFINE_COMPONENT_WITH_SYSTEM(SpawnPointComponent, PlayerManagementSystem)

    SpawnType Type = SpawnType::Player;
};
