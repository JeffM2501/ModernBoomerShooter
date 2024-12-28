#include "components/mobile_object_component.h"
#include "components/transform_component.h"
#include "systems/mobile_object_system.h"
#include "services/model_manager.h"
#include "services/character_manager.h"

#include "raylib.h"
#include "rlgl.h"

ModelInstance* MobComponent::GetModelInstance()
{
    return Instance.get();
}

void MobComponent::SetSpeedFactor(float value) 
{ 
    if (Instance)
        Instance->SetAnimationFPSMultiplyer(value);
}

void MobComponent::OnAddedToObject()
{
    AddToSystem<MobSystem>();
}

void MobComponent::Draw()
{
    auto* transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    if (Instance)
    {
        Instance->Advance(GetFrameTime());
        Instance->Draw(*transform);
    }
    else
    {
        rlPushMatrix();
        rlTranslatef(transform->Position.x, transform->Position.y, transform->Position.z + 0.375f);
        rlRotatef(transform->GetFacing(), 0, 0, 1);
        DrawCube(Vector3Zeros, 0.25f, 0.25f, 0.75f, RED);
        DrawCube(Vector3UnitY * 0.125f + Vector3UnitZ * 0.3f, 0.25f, 0.005f, 0.125f, YELLOW);
        DrawCube(Vector3UnitZ * 0.125f, 0.5f, 0.125f, 0.125f, MAROON);
        DrawCube(Vector3UnitX * 0.3f + Vector3UnitY * 0.125f, 0.125f, 0.4f, 0.125f, PURPLE);
        rlPopMatrix();
    }
}

void MobComponent::OnCreate()
{
    Character = CharacterManager::GetCharacter("guard");

    if (!Character)
        return;

    Instance = ModelManager::GetAnimatedModel(Character->ModelName);
    Instance->Geometry->Geometry.transform = MatrixRotate(Vector3UnitZ, Character->RotationOffset * DEG2RAD);

    SetAnimationState(CharacterAnimationState::Idle);
}

void MobComponent::SetAnimationState(CharacterAnimationState state)
{
    if (Character == nullptr || AnimationState == state || !Character->SequenceNames.contains(state))
        return;

    AnimationState = state;
    Instance->SetSequence(Character->SequenceNames[state]);
}
