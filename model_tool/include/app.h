#pragma once

#include <string>
#include <memory>

#include "raylib.h"

class Panel
{
public:
    std::string Name;
    bool Open = true;

    virtual ~Panel() = default;

    void Show();

protected:
    virtual void OnShowContents() = 0;
};

namespace App
{
    Model& GetModel();

    void LoadModel(const char* filename);

    void RequestQuit();

    void AddPanel(std::unique_ptr<Panel> panel);

    template<class T>
    void AddPanel() { AddPanel(std::make_unique<T>()); }
}

