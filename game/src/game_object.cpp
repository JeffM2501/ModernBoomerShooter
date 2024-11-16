#include "game_object.h"
#include "world.h"
#include "system.h"

GameObject::~GameObject()
{
    if (!WorldPtr)
        return;

    for (auto guid : LinkedSystems)
    {
        auto* system = WorldPtr->GetSystem(guid);
        if (system)
            system->RemoveObject(this);
    }
}

GameObject* GameObject::AddChild()
{
    GameObject* object = Children.emplace_back(std::make_unique<GameObject>(WorldPtr)).get();
    object->ParentPtr = this;
    return object;
}

GameObject::GameObject(World* world)
    :WorldPtr(world)
{
}

GameObject* GameObject::GetParent() const
{
    return ParentPtr;
}

World* GameObject::GetWorld()
{
    return WorldPtr;
}

const World* GameObject::GetWorld() const
{
    return WorldPtr;
}

void GameObject::AddToSystem(size_t systemGUID)
{
    if (!WorldPtr)
        return;

    if (LinkedSystems.find(systemGUID) != LinkedSystems.end())
        return;

    auto* system = WorldPtr->GetSystem(systemGUID);
    if (!system)
        return;

    system->AddObject(this);
    LinkedSystems.insert(systemGUID);
}
