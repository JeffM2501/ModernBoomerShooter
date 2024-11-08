#pragma once

#include <memory>

#include <vector>
#include <unordered_map>

class Component;

class GameObject
{
public:
    std::vector<std::unique_ptr<GameObject>> Children;
    std::unordered_map<uint64_t, Component*> Components;

    GameObject* AddChild();
};