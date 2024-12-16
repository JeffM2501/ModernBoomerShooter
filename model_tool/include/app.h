#pragma once

#include <string>
#include <memory>

#include "raylib.h"

enum class PanelDockingType
{
    Floating = 0,
    LeftDock,
    RightDock,
};

class Panel
{
public:
    std::string Name;
    std::string Icon;

    uint32_t ExtraWindowFlags = 0;

    Vector2 MinSize = { -1. - 1 };

    PanelDockingType DockingType = PanelDockingType::Floating;

    bool Open = true;

    virtual ~Panel() = default;

    void Show();

protected:
    virtual void OnShowContents() = 0;
};

namespace App
{
    Model& GetModel();
    void SetModel(Model& model);

    void LoadModel(const char* filename);

    void RequestQuit();

    void AddPanel(std::unique_ptr<Panel> panel);

    template<class T>
    void AddPanel() { AddPanel(std::make_unique<T>()); }
}

