#pragma once

#include "map/map.h"
#include "rcamera.h"

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
};

