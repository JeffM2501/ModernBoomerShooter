#pragma once

#include "component.h"
#include "game_object.h"
#include "raylib.h"
#include "raymath.h"
#include "services/character_manager.h"

#include <string>
#include <memory>

class ModelInstance;
class AnimatedModelInstance;
struct CharacterInfo;

class MobComponent : public Component
{
public:
    DEFINE_COMPONENT(MobComponent)

    void OnAddedToObject() override;

    void Draw();

    ModelInstance* GetModelInstance();

    void SetSpeedFactor(float value);
    void SetAnimationState(CharacterAnimationState state);

protected:
    void OnCreate() override;

protected:
    std::shared_ptr<AnimatedModelInstance> Instance = nullptr;

    std::shared_ptr<CharacterInfo> Character = nullptr;
    CharacterAnimationState AnimationState = CharacterAnimationState::None;
};
