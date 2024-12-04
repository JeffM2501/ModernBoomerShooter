#pragma once
#include "system.h"

class MobSystem : public System
{
public:
    DEFINE_SYSTEM(MobSystem);

protected:
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;
    void OnRemoveObject(GameObject* object) override;

protected:
};