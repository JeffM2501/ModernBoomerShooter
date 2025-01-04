#pragma once

#include <memory>
#include "game_object.h"

enum class SystemStage
{
    PreUpdate,
    Update,
    PostUpdate,
    Async,
    PreRender,
    Render,
    PostRender,
};

enum class GameState
{
    Empty,
    Loading,
    Playing,
    Closing,
};

class System;
class Scene;

namespace App
{
    void RegisterSystem(SystemStage stage, std::unique_ptr<System> system);

    template<class T>
    inline void RegisterSystem(SystemStage stage)
    {
        App::RegisterSystem(stage, std::move(std::make_unique<T>()));
    }

    System* GetSystem(size_t systemId);

    template<class T>
    inline T* GetSystem()
    {
        return static_cast<T*>(GetSystem(T::GUID()));
    }

    void Init();
    void Cleanup();

    void Reset();

    void NewFrame();

    void AddEventHandler(size_t hash, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token);
    void AddEventHandler(std::string_view name, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token);
    void CallEvent(size_t hash, GameObject* sender, GameObject* target);

    void Quit();

    GameState& GetState();
    Scene& GetScene();
}