#pragma once

#include "raylib.h"

#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>


struct BillboardFrameSequence
{
    std::vector<std::map<float, Rectangle>> Frames;
};

struct BillboardDefintion
{
    std::string TextureName;
    Texture SpriteSheet = { 0 };
    std::unordered_map<std::string, BillboardFrameSequence> Sequences;
};

struct BillboardCharacterInstance
{
    BillboardDefintion& Defintion;

    BillboardFrameSequence* CurrentSequence = nullptr;
    size_t CurrentFrame = 0;
    float AnimationSpeed = 1.0f / 15.0f;

    void Draw(float facingAngle, float cameraAngle, Vector3& position, Camera3D& camera);
};

struct CharacterInfo
{
    virtual ~CharacterInfo() = default;

    std::string Name;
    bool IsBillboard = true;
};

struct BillboardCharacterInfo : public CharacterInfo
{
    BillboardDefintion Defintion;

    std::shared_ptr<BillboardCharacterInstance> GetInstance();
};

namespace CharacterManager
{
    void Init();
    void Cleanup();

    std::shared_ptr<CharacterInfo> GetCharacter(const std::string& name);
    void UnloadAll();
};