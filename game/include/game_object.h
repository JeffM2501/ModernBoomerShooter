#pragma once

#include <memory>

#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <memory>

#include "component.h"

class World;

class GameObject;

class GameObjectLifetimeToken
{
private:
    GameObject* Owner = nullptr;

public:
    GameObjectLifetimeToken(GameObject* owner) : Owner(owner) {}

    using Ptr = std::shared_ptr<GameObjectLifetimeToken>;
    static Ptr Create(GameObject* owner)
    {
        return std::make_shared<GameObjectLifetimeToken>(owner);
    }

    bool IsValid() const { return Owner != nullptr; }

    void Invalidate() { Owner = nullptr; }

    GameObject* GetOwner() { return Owner; }
};

using GameObjectEventHandler = std::function<void(size_t, GameObject*, GameObject*)>;

struct GameObjectEventRecord
{
    GameObjectEventHandler Handler;
    GameObjectLifetimeToken::Ptr LifetimeToken;
};

class GameObject
{
public:
    std::vector<std::unique_ptr<GameObject>> Children;
    std::unordered_map<size_t, std::unique_ptr<Component>> Components;

public:
    GameObject(World* world);
    ~GameObject();

    GameObject* AddChild();

    World* GetWorld();
    const World* GetWorld() const;

    template<class T, typename... Args>
    inline T* AddComponent(Args&&... args)
    {
        auto itr = Components.find(T::TypeID());
        if (itr != Components.end())
            return static_cast<T*>(itr->second.get());

        auto comp = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* ptr = static_cast<T*>(comp.get());

        Components.try_emplace(T::TypeID(), std::move(comp));

        ptr->OnAddedToObject();
        return ptr;
    }

    template<class T>
    inline bool HasComponent()
    {
        return Components.find(T::TypeID()) != Components.end();
    }

    template<class T>
    inline T* GetComponent()
    {
        auto itr = Components.find(T::TypeID());
        if (itr == Components.end())
            return nullptr;

        auto& comp = itr->second;
        return static_cast<T*>(comp.get());
    }

    template<class T>
    inline T& MustGetComponent()
    {
        auto itr = Components.find(T::TypeID());
        if (itr == Components.end())
            return *AddComponent<T>();

        auto& comp = itr->second;
        return *static_cast<T*>(comp.get());
    }

    void AddToSystem(size_t systemGUID);

    GameObjectLifetimeToken::Ptr GetToken() { return Token; }

    void AddEvent(size_t hash, GameObjectEventHandler handler, GameObjectLifetimeToken::Ptr token);
    void AddEvent(std::string_view name, GameObjectEventHandler handler, GameObjectLifetimeToken::Ptr token);

    void CallEvent(size_t hash, GameObject* target = nullptr);
    void CallEvent(std::string_view name, GameObject* target = nullptr);

    void AddFlag(size_t flag) { Flags.insert(flag); }
    void AddFlag(void* flag) { AddFlag(reinterpret_cast<size_t>(flag)); }

    void ClearFlag(size_t flag) { Flags.erase(flag); }
    void ClearFlag(void* flag) { ClearFlag(reinterpret_cast<size_t>(flag)); }

    bool HasFlag(size_t flag) const { return Flags.contains(flag); }
    bool HasFlag(void* flag) const { return HasFlag(reinterpret_cast<size_t>(flag)); }

protected:
    friend World;
    World* WorldPtr = nullptr;
    GameObject* ParentPtr = nullptr;
    std::set<size_t> LinkedSystems;

    GameObjectLifetimeToken::Ptr Token;

    std::unordered_map<size_t, std::vector<GameObjectEventRecord>> EventHandlers;

    std::set<size_t> Flags;

protected:
    GameObject* GetParent() const;
};