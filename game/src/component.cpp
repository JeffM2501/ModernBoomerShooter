#include "component.h"
#include "game_object.h"
#include "world.h"

void Component::AddToSystem(size_t systemGUID)
{
    if (!Owner)
        return;

    Owner->AddToSystem(systemGUID);
}
