#pragma once
#include "system.h"

class MenuRenderSystem : public System
{
public:
    DEFINE_SYSTEM(MenuRenderSystem);

protected:
    void OnUpdate() override;

protected:
    bool InMenu = false;
};