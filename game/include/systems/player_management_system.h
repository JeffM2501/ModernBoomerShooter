#pragma once

#include "system.h"
#include "raylib.h"
#include "raymath.h"

class SpawnPointComponent;
class MapObjectSystem;
class TransformComponent;

class PlayerManagementSystem : public System
{
public:
    DEFINE_SYSTEM(PlayerManagementSystem)

    Vector3 GetPlayerPos() const;
    Vector3 GetPlayerFacing() const;

    float GetPlayerPitch() const;

    static constexpr char PlayerHitWall[] = "PlayerHitWall";
    static constexpr char PlayerHitObstacle[] = "PlayerHitObstacle";

protected:
    void OnSetup() override;
    void OnUpdate() override;
    void OnAddObject(GameObject* object) override;

protected:
    class InputSystem* Input = nullptr;

    float PlayerPitch = 0;

    float PlayerFowardSpeed = 4;
    float PlayerSideStepSpeed = 2;

    SpawnPointComponent* Spawn = nullptr;

    MapObjectSystem* MapObjects = nullptr;

    GameObject* PlayerObject = nullptr;
    TransformComponent* PlayerTransform = nullptr;
};