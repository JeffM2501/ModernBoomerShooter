#include "components/trigger_component.h"

#include <algorithm>

TriggerComponent::TriggerComponent(GameObject* owner)
    : Component(owner), Visualizer(this)
{
}

TriggerComponent::TriggerComponent(GameObject* owner, const Rectangle& bounds) 
    : Component(owner), Bounds(bounds), Visualizer(this)
{
    Visualizer.SetDrawFunctions([this](const Camera&) {DrawDebug(); });
}

static constexpr float DebugFloorHeight = 0.01f;

void TriggerComponent::DrawDebug()
{
    DrawLine3D(Vector3{ Bounds.x, Bounds.y, DebugFloorHeight },
        Vector3{ Bounds.x + Bounds.width, Bounds.y, DebugFloorHeight },
        RED);

    DrawLine3D(Vector3{ Bounds.x + Bounds.width, Bounds.y, DebugFloorHeight },
        Vector3{ Bounds.x + Bounds.width, Bounds.y + Bounds.height, DebugFloorHeight },
        RED);

    DrawLine3D(Vector3{ Bounds.x + Bounds.width, Bounds.y + Bounds.height, DebugFloorHeight },
        Vector3{ Bounds.x, Bounds.y + Bounds.height, DebugFloorHeight },
        RED);

    DrawLine3D(Vector3{ Bounds.x, Bounds.y + Bounds.height, DebugFloorHeight },
        Vector3{ Bounds.x, Bounds.y, DebugFloorHeight },
        RED);
}

void TriggerComponent::AddObject(ObjectLifetimeToken::Ptr token)
{
    if (token->IsValid())
        ConainedObjects.push_back(token);
}

void TriggerComponent::RemovObject(ObjectLifetimeToken::Ptr token)
{
    if (!token->IsValid())
        return;

    auto itr = std::find(ConainedObjects.begin(), ConainedObjects.end(), token);
    if (itr != ConainedObjects.end())
        ConainedObjects.erase(itr);
}

bool TriggerComponent::HasObject(GameObject* object)
{
    for (auto itr = ConainedObjects.begin(); itr != ConainedObjects.end();)
    {
        ObjectLifetimeToken* tokenPtr = itr->get();

        if (!tokenPtr->IsValid())
        {
            ConainedObjects.erase(itr);
        }
        else
        {
            if (tokenPtr->GetOwner() == object)
                return true;

            itr++;
        }
    }
    return false;
}

bool TriggerComponent::HasAnyObjects()
{
    for (auto itr = ConainedObjects.begin(); itr != ConainedObjects.end();)
    {
        ObjectLifetimeToken* tokenPtr = itr->get();

        if (!tokenPtr->IsValid())
        {
            ConainedObjects.erase(itr);
        }
        else
        {
            return true;
        }
    }
    return false;
}
