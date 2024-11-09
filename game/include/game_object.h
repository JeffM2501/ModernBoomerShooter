#pragma once

#include <memory>

#include <vector>
#include <unordered_map>
#include <set>

#include "component.h"

class World;

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

    template<class T>
    inline T* AddComponent()
    {
        auto itr = Components.find(T::TypeID());
        if (itr != Components.end())
            return reinterpret_cast<T*>(itr->second.get());

        Components.emplace(T::TypeID(), std::move(T::Create(this))).second.get();
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

        return reinterpret_cast<T*>(itr.second().get());
    }

    void AddToSystem(size_t systemGUID);

protected:
    friend World;
    World* WorldPtr = nullptr;
    GameObject* ParentPtr = nullptr;
    std::set<size_t> LinkedSystems;

protected:
    GameObject* GetParent() const;
};