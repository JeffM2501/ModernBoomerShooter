#pragma once

#include <set>
#include <string_view>
#include "object_lifetime_token.h"

/*
Systems are classes that are tied to the world that have an update method that is called by the game loop.
Each system is updated in order as defined by how it's registered.
*/

class GameObject;
class Scene;


using SystemHash = std::hash<std::string_view>;

#define DEFINE_SYSTEM(T) \
    T() : System(){} \
    static size_t GUID() { static std::hash<std::string_view> hasher; return hasher(#T);} \
    size_t GetGUID() const override {return T::GUID();} 

#define DEFINE_SYSTM_NO_CONSTRUCTOR(T) \
    static size_t GUID() { static std::hash<std::string_view> hasher; return hasher(#T);} \
    size_t GetGUID() const override {return T::GUID();} 

class System
{
public:
    System() { Token = ObjectLifetimeToken::Create(this); }
    virtual ~System() { Token->Invalidate(); }

    void Init();    // called when the system is created
    void Cleanup(); // called when the system is destroyed

    void Setup();   // all systems and states are loaded

    void Update();  // called when the system is updated

    virtual bool IsReady() { return true; }

    void AddObject(GameObject* object);     // called when an object is added to the system,
    void RemoveObject(GameObject* object);  // called when an object is removed

    virtual size_t GetGUID() const = 0;

    const std::set<GameObject*>& GetSystemObjects() const { return Objects; }

protected:
    virtual void OnInit() {}
    virtual void OnSetup() {}
    virtual void OnCleaup() {}
    virtual void OnUpdate() {}

    virtual void OnAddObject(GameObject* object) {}
    virtual void OnRemoveObject(GameObject* object) {}

protected:
    std::set<GameObject*> Objects;

    ObjectLifetimeToken::Ptr Token;
};