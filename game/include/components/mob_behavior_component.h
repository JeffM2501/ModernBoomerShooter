#pragma once
#include "component.h"
#include "systems/mobile_object_system.h"

class MobBehaviorComponent : public Component
{
public:
    DEFINE_COMPONENT_WITH_SYSTEM(MobBehaviorComponent, MobSystem)

    void Process();

    bool FollowPath = false;
    bool LoopPath = true;

    float MoveSpeed = 2;
    float RotationSpeed = 45;

    std::vector<Vector2> Path;

protected:
    enum class AIState
    {
        Unknown,
        Turning,
        Moving,
        Waiting,
    };

    AIState State = AIState::Unknown;

    float WaitTime = 0;
    size_t CurrentPathIndex = 0;

    Vector3 DesiredPostion = { 0, 0, 0 };
    float DesiredAngle = 0;

protected:
    float GetAngleToPathPoint() const;
    float GetDistanceToPlathPoint() const;
};
