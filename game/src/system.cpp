#include "system.h"
#include <algorithm>

void System::Init()
{
    OnInit();
}

void System::Cleanup()
{
    OnCleaup();
}

void System::Setup()
{
    OnSetup();
}

bool System::Update()
{
    return OnUpdate();
}

void System::AddObject(GameObject* object)
{
    Objects.push_back(object);
}

void System::RemoveObject(GameObject* object)
{
    auto itr = std::find(Objects.begin(), Objects.end(), object);
    if (itr == Objects.end())
        return;

    Objects.erase(itr);
}