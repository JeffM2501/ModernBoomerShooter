#pragma once

#include <memory>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <memory>

#include "component.h"
#include "object_lifetime_token.h"

class World;
class GameObject;

using GameObjectEventHandler = std::function<void(size_t, GameObject*, GameObject*)>;

struct GameObjectEventRecord
{
    GameObjectEventHandler Handler;
    ObjectLifetimeToken::Ptr LifetimeToken;
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
    inline const T* GetComponent() const
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

    ObjectLifetimeToken::Ptr GetToken() { return Token; }

    void AddEventHandler(size_t hash, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token);
    void AddEventHandler(std::string_view name, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token);

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

    ObjectLifetimeToken::Ptr Token;

    std::unordered_map<size_t, std::vector<GameObjectEventRecord>> EventHandlers;

    std::set<size_t> Flags;

protected:
    GameObject* GetParent() const;
};

// utility class to cache a list of components in systems from the game objects they have
// will check if the object has the component and if so, cache it's pointer for ECS-like iteration
template<class T>
class SystemComponentList
{
public:
    std::vector<T*> Components;

    inline T* Add(GameObject* object)
    {
        T* comp = object->GetComponent<T>();
        if (comp)
            Components.push_back(comp);

        return comp;
    }
 
    inline T* MustAdd(GameObject* object)
    {
        T& comp = object->MustGetComponent<T>();
        Components.push_back(&comp);
 
        return &comp;
    }

    inline void Remove(GameObject* object)
    {
        T* comp = object->GetComponent<T>();

        if (comp)
        {
            auto itr = std::find(Components.begin(), Components.end(), comp);
            if (itr != Components.end())
                Components.erase(itr);
        }
    }
};