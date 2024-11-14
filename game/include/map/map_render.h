#pragma once

#include "map/map.h"
#include "raylib.h"

class Raycaster;

class MapRenderer
{
public:
    Camera3D Viepoint = { 0 };
    Map& WorldMap;
    Raycaster& WorldRaycaster;

    MapRenderer(Map& map, Raycaster& caster);
    void Reset();

    void Render();

    void SetViewpoint(Vector3 position, float yaw, float pitch);

    void SetEyeHeight(float height);

private:
    void RenderCell(int x, int y);

private:
    float EyeHeight = 0.5f;

    float MapScale = 1;

    float WallColors[4] = { 1, 0.4f, 0.75f , 0.8f };
    float DefaultExteriorZoneLevel = 0.95f;
    float DefaultIntereorZoneLevel = 0.8f;
};

