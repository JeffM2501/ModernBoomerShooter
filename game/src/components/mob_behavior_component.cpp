#include "components/mob_behavior_component.h"

#include "components/transform_component.h"
#include "components/mobile_object_component.h"
#include "systems/player_management_system.h"
#include "systems/map_object_system.h"
#include "services/game_time.h"
#include "utilities/ai_utils.h"
#include "utilities/collision_utils.h"
#include "game_object.h"

#include "game.h"
#include "scene.h"
#include "map/map.h"

#include "raylib.h"
#include "rlgl.h"

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

    return Vector3Distance(transform->Position, Path[CurrentPathIndex]);
}

MobBehaviorComponent::MobBehaviorComponent(GameObject* owner)
    : Component(owner)
    , Visualizer(this)
{
    Visualizer.SetDrawFunctions([this](const Camera&) 
        {
            DrawSphereWires(DesiredPostion + Vector3UnitZ * 0.125f, 0.125f * 0.5f, 3, 4, FollowPath ? RED : PURPLE);

            if (FollowPath)
            {
                rlPushMatrix();
                rlTranslatef(0, 0, 0.125f);
                for (int i = 0; i < Path.size()-1; i++)
                {
                    DrawLine3D(Path[i], Path[i + 1], WHITE);
                }
                rlPopMatrix();
            }
        });
}

void MobBehaviorComponent::Process()
{
    auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    auto* mob = GetOwner()->GetComponent<MobComponent>();
    
    switch (State)
    {
    case MobBehaviorComponent::AIState::Unknown:
        if (!FollowPath || Path.empty())
        {
            State = AIState::Waiting;
            if (mob)
            {
                mob->SetSpeedFactor(1);
                mob->SetAnimationState(CharacterAnimationState::Idle);
            }
        }
        else
        {
            DesiredPostion = Vector3{ Path[CurrentPathIndex].x, Path[CurrentPathIndex].y, 0 };
            State = AIState::Moving;
            if (mob)
            {
                mob->SetSpeedFactor(MoveSpeed);
                mob->SetAnimationState(CharacterAnimationState::Walking);
            }
        }
        break;

    case MobBehaviorComponent::AIState::Moving:
    {
        bool done = false;

        float maxMoveThisFrame = MoveSpeed * GameTime::GetDeltaTime();
        float maxRotationThisFrame = RotationSpeed * GameTime::GetDeltaTime();

        Vector3 desiredMotion = AIUtils::MoveTo(transform->Position, transform->Forward, DesiredPostion, maxMoveThisFrame, maxRotationThisFrame);

        if (Vector3LengthSqr((desiredMotion + transform->Position) - DesiredPostion) < 0.001f)
            done = true;

        bool hitSomething = false;
        if (App::GetScene().GetMap().MoveEntity(transform->Position, desiredMotion, 0.25f))
        {
            hitSomething = true;
            done = !FollowPath;
        }

        transform->Position += desiredMotion;

        if (mob)
            mob->SetSpeedFactor(MoveSpeed);

        App::GetSystem<MapObjectSystem>()->CheckTriggers(GetOwner(), 0.25f, hitSomething);

        if (done)
        {
            if (FollowPath && !Path.empty())
            {
                ++CurrentPathIndex;
                if (CurrentPathIndex >= Path.size())
                    CurrentPathIndex = 0;

                DesiredPostion = Vector3{ Path[CurrentPathIndex].x, Path[CurrentPathIndex].y, 0 };
            }
            else
            {
                if (hitSomething)
                {
                    transform->SetFacing(transform->GetFacing() + float(GetRandomValue(180 - 30, 180 + 30)));
                    DesiredPostion = transform->Position + transform->Forward * float(GetRandomValue(1, 3));
                }
                else
                {
                    State = AIState::Waiting;
                    WaitTime = float(GetRandomValue(2, 10));
                    if (mob)
                    {
                        mob->SetSpeedFactor(1);
                        mob->SetAnimationState(CharacterAnimationState::Idle);
                    }
                }
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
            State = AIState::Moving;

            if (mob)
            {
                mob->SetSpeedFactor(MoveSpeed);
                mob->SetAnimationState(CharacterAnimationState::Walking);
            }

            float angle = transform->GetFacing() + float(GetRandomValue(180 - 30, 180 + 30));
            Vector3 newVec = { cosf((angle + 90) * DEG2RAD), sinf((angle + 90) * DEG2RAD), 0 };
            DesiredPostion = transform->Position + newVec * float(GetRandomValue(4, 10));
        }
        break;
    }
}
