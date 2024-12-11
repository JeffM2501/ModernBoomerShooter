#pragma once
#include "component.h"
#include "systems/mobile_object_system.h"

#include "utilities/debug_draw_utility.h"

class MobBehaviorComponent : public Component
{
public:
    DEFINE_COMPONENT_WITH_SYSTEM_NO_CONSTRUCTOR(MobBehaviorComponent, MobSystem);

    MobBehaviorComponent(GameObject* owner);

    void Process();

    bool FollowPath = false;
    bool LoopPath = true;

    float MoveSpeed = 2;
    float RotationSpeed = 180;

    std::vector<Vector3> Path;

protected:
    enum class AIState
    {
        Unknown,
        Moving,
        Waiting,
    };

    AIState State = AIState::Unknown;

    float WaitTime = 0;
    size_t CurrentPathIndex = 0;

    Vector3 DesiredPostion = { 0, 0, 0 };
    float DesiredAngle = 0;

    DebugDrawUtility::DebugDraw Visualizer;

protected:
    float GetAngleToPathPoint() const;
    float GetDistanceToPlathPoint() const;
};
