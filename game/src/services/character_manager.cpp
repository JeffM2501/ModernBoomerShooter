#include "services/character_manager.h"
#include "services/table_manager.h"
#include "services/texture_manager.h"
#include "utilities/string_utils.h"
#include "world.h"

static constexpr char CharacterManifest[] = "character_manifest";

const Table* CharacterManifestTable = nullptr;

namespace CharacterManager
{
    static std::unordered_map<std::string, std::shared_ptr<CharacterInfo>> CharacterCache;


    std::shared_ptr<CharacterInfo> LoadCharacter(const std::string& key)
    {
        auto* characterTable = CharacterManifestTable->GetFieldAsTable(key);
        if (!characterTable)
            return;

        if (characterTable->GetField("type") == "billboard")
        {
            auto character = std::make_shared<BillboardCharacterInfo>();
            character->Name = key;
            character->IsBillboard = true;
            character->Defintion.TextureName = characterTable->GetField("sheet");
            character->Defintion.SpriteSheet = TextureManager::GetTexture(character->Defintion.TextureName);

            auto sheetLayout = StringUtils::SplitString(characterTable->GetField("sheet"), ",");
            int sheetTilesX = 1;
            int sheetTilesY = 1;

            if (sheetLayout.size() > 0 && !sheetLayout[0].empty())
                sheetTilesX = atoi(sheetLayout[0].c_str());

            if (sheetLayout.size() > 1 && !sheetLayout[1].empty())
                sheetTilesY = atoi(sheetLayout[1].c_str());

            float sheetFrameX = character->Defintion.SpriteSheet.width / float(sheetTilesX);
            float sheetFrameY = character->Defintion.SpriteSheet.height / float(sheetTilesY);

            for (auto& sequenceName : StringUtils::SplitString(characterTable->GetField("sequences"), ","))
            {
                auto sequenceList = characterTable->GetField(sequenceName);
                if (sequenceList.empty())
                    continue;

                BillboardFrameSequence sequence;

                for (auto& frameset : StringUtils::SplitString(sequenceList, ":"))
                {
                    float angleDelta = frameset.size() / 360.0f;
                    sequence.Frames.push_back(std::map<float, Rectangle>());

                    for (float a = 0; a <= 360; a += angleDelta)
                    {
                        int frame = atoi(frameset.c_str());

                        int frameY = frame % sheetTilesX;
                        int frameX = frame - (frameY * sheetTilesX);

                        Rectangle rect = { frameX * sheetFrameX , frameY * sheetFrameY, sheetFrameX, sheetFrameY };
                        sequence.Frames.back().insert_or_assign(a, rect);
                    }
                }

                character->Defintion.Sequences.insert_or_assign(sequenceName, sequence);
            }
            CharacterCache.insert_or_assign(key, character);

            return character;
        }

        return nullptr;
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
    }

    void UnloadAll()
    {
    }
};