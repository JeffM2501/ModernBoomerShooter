#pragma once

#include <set>
#include <string_view>

class GameObject;
class World;

using SystemHash = std::hash<std::string_view>;

#define DEFINE_SYSTEM(T) \
    T(World* world) : System(world){} \
    static size_t GUID() { static std::hash<std::string_view> hasher; return hasher(#T);} \
    size_t GetGUID() const override {return T::GUID();} 

#define DEFINE_SYSTM_NO_CONSTRUCTOR(T) \
    static size_t GUID() { static std::hash<std::string_view> hasher; return hasher(#T);} \
    size_t GetGUID() const override {return T::GUID();} 

class System
{
public:
    System(World* world) : WorldPtr(world) {};
    virtual ~System() = default;

    void Init();
    void Cleanup();

    void Setup(); // all systems and states are loaded

    void Update();

    void AddObject(GameObject* object);
    void RemoveObject(GameObject* object);

    virtual size_t GetGUID() const = 0;

protected:
    virtual void OnInit() {}
    virtual void OnSetup() {}
    virtual void OnCleaup() {}
    virtual void OnUpdate() {}

    virtual void OnAddObject(GameObject* object) {}
    virtual void OnRemoveObject(GameObject* object) {}

protected:
    std::set<GameObject*> Objects;
    World* WorldPtr = nullptr;
};