#include "components/mob_behavior_component.h"

#include "components/transform_component.h"
#include "systems/player_management_system.h"
#include "services/game_time.h"
#include "utilities/collision_utils.h"
#include "world.h"
#include "map/map.h"

#include "raylib.h"

template <typename T> 
inline int sgn(T val) 
{
    return (T(0) < val) - (val < T(0));
}

void MobBehaviorComponent::Process()
{
    auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    auto* world = GetOwner()->GetWorld();
    
    switch (State)
    {
    case MobBehaviorComponent::AIState::Turning:
        {
        bool atTarget = false;
        float maxRotationThisFrame = RotationSpeed * GameTime::GetDeltaTime();

        float delta = DesiredAngle - transform->Facing;
        if (fabsf(delta) <= maxRotationThisFrame)
        {
            transform->Facing = DesiredAngle;
            atTarget = true;
        }
        else
        {
            transform->Facing += sgn(delta) * maxRotationThisFrame;
        }

        if (atTarget)
        {
            State = AIState::Moving;
            DesiredPostion = transform->Position + (transform->GetForward() * float(GetRandomValue(1, 6)));
        }
    }
        break;

    case MobBehaviorComponent::AIState::Moving:
    {
        bool done = false;

        float maxMoveThisFrame = MoveSpeed * GameTime::GetDeltaTime();

        Vector3 position = transform->Position;
        Vector3 desiredMotion = DesiredPostion - transform->Position;

        float distanceToTarget = Vector3Length(desiredMotion);

        // we would overshoot this frame so clamp
        if (maxMoveThisFrame > distanceToTarget)
        {
            maxMoveThisFrame = distanceToTarget;
            done = true;
        }

        desiredMotion = (desiredMotion/ distanceToTarget) * maxMoveThisFrame;
       
        if (world->GetMap().MoveEntity(position, desiredMotion, 0.25f))
        {
            done = true;
        }
        
        transform->Position = position + desiredMotion;

        if (done)
        {
            State = AIState::Waiting;
            WaitTime = float(GetRandomValue(1, 3));
        }
    }
        break;

    default:
    case MobBehaviorComponent::AIState::Waiting:
        WaitTime -= GameTime::GetDeltaTime();
        if (WaitTime <= 0)
        {
            WaitTime = 0;
            State = AIState::Turning;

            DesiredAngle = float(GetRandomValue(0, 3) * 90);
            CollisionUtils::SetUnitAngleDeg(DesiredAngle);
            CollisionUtils::SetUnitAngleDeg(transform->Facing);

            // ensure that our desired angle has the same sign as our facing angle so we can short rot
            if (DesiredAngle >= 0 != transform->Facing >= 0)
            {
                if (DesiredAngle >= 0)
                    DesiredAngle -= 360.0f;
                else
                    DesiredAngle += 360.0f;
            }
        }
        break;
    }
}
