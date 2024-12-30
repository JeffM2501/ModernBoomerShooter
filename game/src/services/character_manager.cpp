#include "services/character_manager.h"
#include "services/table_manager.h"
#include "services/texture_manager.h"
#include "utilities/collision_utils.h"
#include "utilities/string_utils.h"
#include "world.h"

static constexpr char CharacterManifest[] = "character_manifest";

const Table* CharacterManifestTable = nullptr;

namespace CharacterManager
{
    static std::unordered_map<std::string, std::shared_ptr<CharacterInfo>> CharacterCache;


    std::shared_ptr<CharacterInfo> LoadCharacter(const std::string& key)
    {
        if (!CharacterManifestTable)
            return nullptr;

        auto* characterTable = CharacterManifestTable->GetFieldAsTable(key);
        if (!characterTable)
            return nullptr;

        auto character = std::make_shared<CharacterInfo>();
        character->Name = key;

        character->ModelName = characterTable->GetField("model");

        if (characterTable->HasField("rotation_offset"))
            character->RotationOffset = float(atof(characterTable->GetField("rotation_offset").data()));

        if (characterTable->HasField("idle"))
            character->SequenceNames.insert_or_assign(CharacterAnimationState::Idle, std::string(characterTable->GetField("idle")));

        if (characterTable->HasField("walk"))
            character->SequenceNames.insert_or_assign(CharacterAnimationState::Walking, std::string(characterTable->GetField("walk")));

        if (characterTable->HasField("run"))
            character->SequenceNames.insert_or_assign(CharacterAnimationState::Running, std::string(characterTable->GetField("run")));

        if (characterTable->HasField("turn"))
            character->SequenceNames.insert_or_assign(CharacterAnimationState::Turning, std::string(characterTable->GetField("turn")));
        
        if (characterTable->HasField("y_up"))
            character->IsYUp = characterTable->GetField("y_up") != "0";

        CharacterCache.insert_or_assign(key, character);

        return character;
    }

    static bool Preload = true;

    void Init()
    {
        CharacterManifestTable = TableManager::GetTable(BootstrapTable)->GetFieldAsTable(CharacterManifest);

        if (!CharacterManifestTable)
            return;

        if (Preload)
        {
            for (const auto& [key, file] : *CharacterManifestTable)
            {
                LoadCharacter(key);
            }
        }
    }

    void Cleanup()
    {

    }

    std::shared_ptr<CharacterInfo> GetCharacter(const std::string& name)
    {
        auto itr = CharacterCache.find(name);
        if (itr != CharacterCache.end())
            return itr->second;

        return LoadCharacter(name);
    }

    void UnloadAll()
    {
    }
};