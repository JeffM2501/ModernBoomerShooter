#pragma once

#include "raylib.h"

#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>


enum class CharacterAnimationState
{
    None,
    Idle,
    Walking,
    Running,
    Turning
};


struct CharacterInfo
{
    virtual ~CharacterInfo() = default;

    std::string Name;
    std::string ModelName;
    float RotationOffset = 0;
    std::unordered_map<CharacterAnimationState, std::string> SequenceNames;
};

namespace CharacterManager
{
    void Init();
    void Cleanup();

    std::shared_ptr<CharacterInfo> GetCharacter(const std::string& name);
    void UnloadAll();
};