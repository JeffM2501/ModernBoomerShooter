#include "components/mobile_object_component.h"
#include "components/transform_component.h"
#include "systems/mobile_object_system.h"

#include "raylib.h"
#include "rlgl.h"

void MobComponent::OnAddedToObject()
{
    AddToSystem<MobSystem>();
}

void MobComponent::Draw()
{
    auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    rlPushMatrix();
    rlTranslatef(transform->Position.x, transform->Position.y, transform->Position.z + 0.375f);
    rlRotatef(transform->GetFacing(), 0, 0, 1);
    DrawCube(Vector3Zeros, 0.25f, 0.25f, 0.75f, RED);
    DrawCube(Vector3UnitY * 0.125f + Vector3UnitZ * 0.3f, 0.25f, 0.005f, 0.125f, YELLOW);
    DrawCube(Vector3UnitZ * 0.125f, 0.5f, 0.125f, 0.125f, MAROON);
    DrawCube(Vector3UnitX * 0.3f + Vector3UnitY * 0.125f, 0.125f, 0.4f, 0.125f, PURPLE);
    rlPopMatrix();
}
