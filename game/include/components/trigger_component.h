#pragma once

#include "component.h"
#include "game_object.h"

#include "raylib.h"
#include "map/map.h"

#include "systems/map_object_system.h"
#include "utilities/debug_draw_utility.h"

class TriggerComponent : public Component
{
public:
    DEFINE_COMPONENT_WITH_SYSTEM_NO_CONSTRUCTOR(TriggerComponent, MapObjectSystem)

    TriggerComponent(GameObject* owner);
    TriggerComponent(GameObject* owner, const Rectangle& bounds);

    Rectangle Bounds = { 0 };
    int TriggerId = 0;

    static constexpr char TriggerEnter[] = "TriggerComponentEnter";
    static constexpr char TriggerExit[] = "TriggerComponentExit";

    void AddObject(ObjectLifetimeToken::Ptr token);
    void RemovObject(ObjectLifetimeToken::Ptr token);

    bool HasObject(GameObject* object);
    bool HasAnyObjects();

protected:
    void DrawDebug();

protected:
    std::vector<ObjectLifetimeToken::Ptr> ConainedObjects;

    DebugDrawUtility::DebugDraw Visualizer;
};