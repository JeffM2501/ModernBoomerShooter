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

void System::Update()
{
    OnUpdate();
}

void System::AddObject(GameObject* object)
{
    Objects.insert(object);
    OnAddObject(object);
}

void System::RemoveObject(GameObject* object)
{
    Objects.erase(object);
}