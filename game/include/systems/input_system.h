#pragma once

#include "system.h"

class InputSystem : public System
{
public:
    DEFINE_SYSTEM(InputSystem)

protected:
    void OnUpdate() override;
};