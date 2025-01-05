#include "systems/audio_system.h"

#include "services/table_manager.h"
#include "services/resource_manager.h"
#include "services/global_vars.h"

#include <unordered_map>
#include <string_view>

void AudioSystem::OnInit()
{
    if (!IsAudioDeviceReady())
    {
        AudioReady = false;
        AudioLoaderThread = std::thread([]()
            {
                InitAudioDevice();
                SetMasterVolume(GlobalVars::MasterVolume);
            });
    }
      
    
}

void AudioSystem::OnSetup()
{
}

void AudioSystem::OnUpdate()
{
    if (!AudioReady)
    {
        if (!AudioManifestTable)
            AudioManifestTable = TableManager::GetTable(BootstrapTable)->GetFieldAsTable("audio_manifest");

        if (AudioLoaderThread.joinable())
        {
            AudioLoaderThread.join();
            AudioReady = true;
        }
    }

    if (!AudioReady)
        return;

    // TODO handle music updates
}

void AudioSystem::OnCleaup()
{
    if (!AudioReady)
    {
        AudioLoaderThread.join();
        AudioReady = true;
    }
}

SoundInstance::Ptr AudioSystem::GetSound(const std::string& name)
{
    auto itr = LoadedSounds.find(name);
    if (itr != LoadedSounds.end())
        return itr->second;

    if (!AudioManifestTable || !AudioManifestTable->contains(name))
        return nullptr;

    std::string_view file = AudioManifestTable->GetField(name);
    auto resource = ResourceManager::OpenResource(file);
    if (!resource)
        return nullptr;

    Wave wave = LoadWaveFromMemory(GetFileExtension(file.data()), resource->DataBuffer, int(resource->DataSize));
    ResourceManager::ReleaseResource(resource);

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    if (!IsSoundValid(sound))
        return nullptr;

    SoundInstance::Ptr instance = std::make_shared<SoundInstance>(sound);
    LoadedSounds.insert_or_assign(name, instance);
    return instance;
}

int SoundInstance::Play()
{
    int instnace = FindFreeSoundInstance();

    PlaySound(GetSound(instnace));

    return instnace;
}

bool SoundInstance::IsPlaying(int instanceID)
{
    return IsSoundPlaying(GetSound(instanceID));
}

void SoundInstance::Stop(int instanceID)
{
    Sound sound = GetSound(instanceID);
    if (IsSoundPlaying(sound))
        StopSound(sound);
}

void SoundInstance::StopAll()
{
    for (auto& alias : Aliases)
    {
        if (IsSoundPlaying(alias))
            StopSound(alias);
    }

    if (IsSoundPlaying(SourceSound))
        StopSound(SourceSound);
}

const Sound& SoundInstance::GetSound(int instanceID) const
{
    if (instanceID == 0)
        return SourceSound;

    return Aliases[instanceID - 1];
}

int SoundInstance::FindFreeSoundInstance()
{
    if (!IsSoundPlaying(SourceSound))
        return 0;

    int index = 1;
    for (auto& alias : Aliases)
    {
        if (!IsSoundPlaying(alias))
            return index;

        index++;
    }
    Aliases.push_back(LoadSoundAlias(SourceSound));
    return int(Aliases.size());
}

SoundInstance::SoundInstance(Sound sound) : SourceSound(sound)
{
}

SoundInstance::~SoundInstance()
{
    for (auto& alias : Aliases)
    {
        if (IsSoundPlaying(alias))
            StopSound(alias);

        UnloadSoundAlias(alias);
    }

    if (IsSoundPlaying(SourceSound))
        StopSound(SourceSound);
    UnloadSound(SourceSound);
}
