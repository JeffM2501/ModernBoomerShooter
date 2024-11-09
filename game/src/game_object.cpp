#include "game_object.h"

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
