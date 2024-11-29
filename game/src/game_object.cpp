#include "game_object.h"
#include "world.h"
#include "system.h"

static std::hash<std::string_view> StringHasher;

GameObject::~GameObject()
{
    Token->Invalidate();

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
    Token = GameObjectLifetimeToken::Create(this);
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

void GameObject::AddEvent(size_t hash, GameObjectEventHandler handler, GameObjectLifetimeToken::Ptr token)
{
    auto itr = EventHandlers.find(hash);
    if (itr == EventHandlers.end())
    {
        itr = EventHandlers.insert_or_assign(hash, std::vector<GameObjectEventRecord>()).first;
    }

    itr->second.emplace_back(GameObjectEventRecord{ handler, token });
}

void GameObject::AddEvent(std::string_view name, GameObjectEventHandler handler, GameObjectLifetimeToken::Ptr token)
{
    AddEvent(StringHasher(name), handler, token);
}

void GameObject::CallEvent(size_t hash, GameObject* target)
{
    auto itr = EventHandlers.find(hash);
    if (itr == EventHandlers.end())
        return;

    for (std::vector<GameObjectEventRecord>::iterator eventItr = itr->second.begin(); eventItr != itr->second.end();)
    {
        if (eventItr->LifetimeToken->IsValid())
        {
            eventItr->Handler(hash, this, target);
            eventItr++;
        }
        else
        {
            eventItr = itr->second.erase(eventItr);
        }
    }
}

void GameObject::CallEvent(std::string_view name, GameObject* target)
{
    CallEvent(StringHasher(name), target);
}
