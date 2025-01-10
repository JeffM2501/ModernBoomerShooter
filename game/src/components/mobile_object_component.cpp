#include "components/mobile_object_component.h"
#include "components/transform_component.h"
#include "systems/mobile_object_system.h"
#include "services/model_manager.h"
#include "services/character_manager.h"
#include "services/texture_manager.h"

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

    rlPushMatrix();
    rlTranslatef(transform->Position.x, transform->Position.y, transform->Position.z + 0.01f);
    rlRotatef(transform->GetFacing(), 0, 0, 1);
    if (IsTextureValid(ShadowTexture))
    {
        rlBegin(RL_QUADS);

        float shadowSize = 0.45f;
        float shadowAlpha = 0.25f;

        rlSetTexture(ShadowTexture.id);

        rlNormal3f(0, 0, 1);
        rlColor4f(1, 1, 1, shadowAlpha);

        rlTexCoord2f(0, 0);
        rlVertex3f(-shadowSize, -shadowSize, 0);

        rlTexCoord2f(1, 0);
        rlVertex3f(shadowSize, -shadowSize, 0);

        rlTexCoord2f(1, 1);
        rlVertex3f(shadowSize, shadowSize, 0);

        rlTexCoord2f(0, 1);
        rlVertex3f(-shadowSize, shadowSize, 0);

        rlEnd();
        rlSetTexture(-1);
    }
    rlPopMatrix();
}

void MobComponent::OnCreate()
{
    Character = CharacterManager::GetCharacter("walker");

    if (!Character)
        return;

    Instance = ModelManager::GetAnimatedModel(Character->ModelName);

    Instance->Geometry->OrientationTransform = MatrixIdentity();

    Instance->Geometry->OrientationTransform = MatrixRotate(Vector3UnitZ, Character->RotationOffset * DEG2RAD);
    if (Character->IsYUp)
        Instance->Geometry->OrientationTransform = MatrixMultiply(Instance->Geometry->OrientationTransform, MatrixRotate(Vector3UnitX, -90 * DEG2RAD));

    SetAnimationState(CharacterAnimationState::Idle);
    if (!Character->ShadowTexture.empty())
        ShadowTexture = TextureManager::GetTexture(Character->ShadowTexture);
}

void MobComponent::SetAnimationState(CharacterAnimationState state)
{
    if (Character == nullptr || AnimationState == state || !Character->SequenceNames.contains(state))
        return;

    AnimationState = state;
    Instance->SetSequence(Character->SequenceNames[state]);
}
