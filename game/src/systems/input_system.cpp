#include "systems/input_system.h"

#include "raylib.h"
#include "world.h"

void InputSystem::OnUpdate()
{
    if (WindowShouldClose())
        WorldPtr->Quit();      
}

