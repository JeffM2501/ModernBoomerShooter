#pragma once
#include "component.h"
#include "systems/mobile_object_system.h"

class MobBehaviorComponent : public Component
{
public:
    DEFINE_COMPONENT_WITH_SYSTEM(MobBehaviorComponent, MobSystem)

    void Process();

protected:
    enum class AIState
    {
        Turning,
        Moving,
        Waiting,
    };

    AIState State = AIState::Waiting;

    float WaitTime = 0;

    Vector3 DesiredPostion = { 0, 0, 0 };
    float DesiredAngle = 0;

    float MoveSpeed = 2;
    float RotationSpeed = 45;
};
