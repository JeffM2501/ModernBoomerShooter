#pragma once

#include <memory>

#include <vector>
#include <unordered_map>

class Component;
class World;

class GameObject
{
public:
    std::vector<std::unique_ptr<GameObject>> Children;
    std::unordered_map<uint64_t, Component*> Components;

public:
    GameObject(World* world);

    GameObject* AddChild();

    World* GetWorld();
    const World* GetWorld() const;

protected:
    friend World;
    World* WorldPtr = nullptr;
    GameObject* ParentPtr = nullptr;


protected:
    GameObject* GetParent() const;
};