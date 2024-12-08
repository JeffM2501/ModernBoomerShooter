#include "components/mob_behavior_component.h"

#include "components/transform_component.h"
#include "systems/player_management_system.h"
#include "systems/map_object_system.h"
#include "services/game_time.h"
#include "utilities/collision_utils.h"
#include "game_object.h"

#include "world.h"
#include "map/map.h"

#include "raylib.h"

template <typename T> 
inline int sgn(T val) 
{
    return (T(0) < val) - (val < T(0));
}

void MatchAngleSign(float& source, float destination)
{
    if (source >= 0 != destination >= 0)
    {
        if (source >= 0)
            source -= 360.0f;
        else
            source += 360.0f;
    }
}

float MobBehaviorComponent::GetAngleToPathPoint() const
{
    const auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return 0;

    float angle =  atan2f(transform->Position.y - Path[CurrentPathIndex].y, transform->Position.x - Path[CurrentPathIndex].x) * RAD2DEG + 90;

    CollisionUtils::SetUnitAngleDeg(angle);

    return angle;
}

float MobBehaviorComponent::GetDistanceToPlathPoint() const
{
    const auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return 0;

    return Vector2Distance(Vector2(transform->Position.x, transform->Position.y), Path[CurrentPathIndex]);
}

void MobBehaviorComponent::Process()
{
    auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    auto* world = GetOwner()->GetWorld();
    
    switch (State)
    {
    case MobBehaviorComponent::AIState::Unknown:
        if (!FollowPath || Path.empty())
        {
            State = AIState::Waiting;
        }
        else
        {
            DesiredAngle = GetAngleToPathPoint();
            CollisionUtils::SetUnitAngleDeg(transform->Facing);
            State = AIState::Turning;
        }
        break;

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
            if (FollowPath && !Path.empty())
            {
                DesiredPostion = Vector3{ Path[CurrentPathIndex].x, Path[CurrentPathIndex].y, 0 };
            }
            else
            {
                DesiredPostion = transform->Position + (transform->GetForward() * float(GetRandomValue(1, 6)));
            }
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
       
        bool hitSomething = false;
        if (world->GetMap().MoveEntity(position, desiredMotion, 0.25f))
        {
            hitSomething = true;
            done = !FollowPath;
        }
        
        transform->Position = position + desiredMotion;

        GetOwner()->GetWorld()->GetSystem<MapObjectSystem>()->CheckTriggers(GetOwner(), 0.25f, hitSomething);

        if (done)
        {
            if (FollowPath && !Path.empty())
            {
                ++CurrentPathIndex;
                if (CurrentPathIndex >= Path.size())
                    CurrentPathIndex = 0;

                State = AIState::Turning;
                CollisionUtils::SetUnitAngleDeg(transform->Facing);
                DesiredAngle = GetAngleToPathPoint();
                MatchAngleSign(DesiredAngle, transform->Facing);
            }
            else
            {
                State = AIState::Waiting;
                WaitTime = float(GetRandomValue(1, 3));
            }
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
            MatchAngleSign(DesiredAngle, transform->Facing);
        }
        break;
    }
}
