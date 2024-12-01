#pragma once

#include "system.h"

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include "raylib.h"

class Table;

class SoundInstance
{
public:
    int Play();
    bool IsPlaying(int instanceID);
    void Stop(int instanceID);
    void StopAll();

    using Ptr = std::shared_ptr<SoundInstance>;

    SoundInstance(Sound sound);
    ~SoundInstance();
private:
    Sound SourceSound = { 0 };
    std::vector<Sound> Aliases;

private:
    int FindFreeSoundInstance();
    const Sound& GetSound(int instanceID) const;
};

class AudioSystem : public System
{
public:
    DEFINE_SYSTEM(AudioSystem)

    SoundInstance::Ptr GetSound(const std::string& name);
    Music GetMusic(const std::string& name);
    
protected:
    void OnInit() override;
    void OnSetup() override;
    void OnUpdate() override;

private:
    Sound* LoadSound(const std::string& name, const std::string_view& file);

private:
    std::unordered_map<std::string, SoundInstance::Ptr> LoadedSounds;
    const Table* AudioManifestTable = nullptr;
};