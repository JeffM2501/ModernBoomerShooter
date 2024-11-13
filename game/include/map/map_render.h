#pragma once

#include "map/map.h"
#include "raylib.h"

class MapRenderer
{
public:
    Camera3D Viepoint = { 0 };
    Map& WorldMap;

    MapRenderer(Map& map);
    void Reset();

    void Render();

    void SetViewpoint(Vector3 position, float yaw, float pitch);

    void SetEyeHeight(float height);

private:
    void RenderCell(int x, int y);

private:
    float EyeHeight = 1;

    float MapScale = 2;

    float WallColors[4] = { 1, 0.4f, 0.75f , 0.8f };
    float DefaultExteriorZoneLevel = 0.95f;
    float DefaultIntereorZoneLevel = 0.8f;
};

