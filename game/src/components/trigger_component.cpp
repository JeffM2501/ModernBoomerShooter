#include "components/trigger_component.h"

#include <algorithm>

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
