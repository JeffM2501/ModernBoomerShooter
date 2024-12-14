#pragma once
#include "raylib.h"

#include "app.h"

class ModelInfoPanel : public Panel
{
public:
    ModelInfoPanel();

protected:
    void OnShowContents() override;
};