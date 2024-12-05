#include "systems/mobile_object_system.h"

#include "components/transform_component.h"
#include "utilities/collision_utils.h"

void MobSystem::OnUpdate()
{
    // do AI updates

    for (auto& mob : Objects)
    {
        auto* transform = mob->GetComponent<TransformComponent>();
        if (!transform)
            continue;

        constexpr float RotSpeed = 45;
        transform->Facing += GetFrameTime() * RotSpeed;
        CollisionUtils::SetUnitAngleDeg(transform->Facing);

        transform->Position += transform->GetForward() * GetFrameTime() * 1;
    }
}

void MobSystem::OnAddObject(GameObject* object)
{

}

void MobSystem::OnRemoveObject(GameObject* object)
{

}
