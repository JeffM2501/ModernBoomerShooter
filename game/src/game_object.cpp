#include "game_object.h"
#include "scene.h"
#include "system.h"

#include "game.h"

static std::hash<std::string_view> StringHasher;

GameObject::~GameObject()
{
    Token->Invalidate();

    for (auto guid : LinkedSystems)
    {
        auto* system = App::GetSystem(guid);
        if (system)
            system->RemoveObject(this);
    }
}

GameObject* GameObject::AddChild()
{
    GameObject* object = Children.emplace_back(std::make_unique<GameObject>()).get();
    object->ParentPtr = this;
    return object;
}

GameObject::GameObject()
{
    Token = ObjectLifetimeToken::Create(this);
}

GameObject* GameObject::GetParent() const
{
    return ParentPtr;
}

void GameObject::AddToSystem(size_t systemGUID)
{
    auto* system = App::GetSystem(systemGUID);
    if (!system)
        return;

    LinkedSystems.insert(systemGUID);

    system->AddObject(this);
}

void GameObject::AddEventHandler(size_t hash, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token)
{
    auto itr = EventHandlers.find(hash);
    if (itr == EventHandlers.end())
    {
        itr = EventHandlers.insert_or_assign(hash, std::vector<GameObjectEventRecord>()).first;
    }

    itr->second.emplace_back(GameObjectEventRecord{ handler, token });
}

void GameObject::AddEventHandler(std::string_view name, GameObjectEventHandler handler, ObjectLifetimeToken::Ptr token)
{
    AddEventHandler(StringHasher(name), handler, token);
}

void GameObject::CallEvent(size_t hash, GameObject* target)
{
    App::CallEvent(hash, this, target);

    auto itr = EventHandlers.find(hash);
    if (itr == EventHandlers.end())
        return;

    for (std::vector<GameObjectEventRecord>::iterator eventItr = itr->second.begin(); eventItr != itr->second.end();)
    {
        if (eventItr->LifetimeToken->IsValid())
        {
            eventItr->Handler(hash, this, target);
            ++eventItr;
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
