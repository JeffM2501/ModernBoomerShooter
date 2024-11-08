#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

class System;
class GameObject;
class World
{
public:

protected:
    std::vector<System*> SystemUpdateList;
    std::unordered_map<uint64_t, System*> Systems;

    std::unique_ptr<GameObject> RootObject;
};