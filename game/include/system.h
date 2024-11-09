#pragma once

#include <vector>

class GameObject;

class System
{
public:
    void Init();
    void Cleanup();

    void Setup(); // all systems and states are loaded

    bool Update();

    void AddObject(GameObject* object);
    void RemoveObject(GameObject* object);

    virtual uint64_t GetGUID() = 0;

protected:
    virtual void OnInit() {}
    virtual void OnSetup() {}
    virtual void OnCleaup() {}
    virtual bool OnUpdate() { return false; }

    virtual void OnAddObject(GameObject* object) {}
    virtual void OnRemoveObject(GameObject* object) {}

protected:
    std::vector<GameObject*> Objects;
};