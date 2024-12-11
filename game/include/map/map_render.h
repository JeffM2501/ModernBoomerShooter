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

    void SetViewpoint(Vector3 position, float pitch, const Vector3& facing);

    void SetEyeHeight(float height);

    Shader& GetWorldShader() { return WorldShader; }

private:

    struct AmbientOcclusionVertexValue
    {
        int AOValue = 0;
        int ConveredValue = 0;
    };

    struct AmbientOcclusionCellValues
    {
        AmbientOcclusionVertexValue Values[4];
    };

    void RenderCell(int x, int y);
    void RenderDoor(int x, int y, Color floorColor, const AmbientOcclusionCellValues& aoInfo);
    void DrawDoorPanelXAlligned(Color floorColor, Rectangle& uv, const AmbientOcclusionCellValues& aoInfo);
    void DrawDoorPanelYAlligned(Color floorColor, Rectangle& uv, const AmbientOcclusionCellValues& aoInfo);

    void RenderFloor(int x, int y, Color floorColor, Color exteriorFloorColor, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo);
    void RenderCeiling(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo);
    void RenderNorthWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo);
    void RenderSouthWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo);
    void RenderEastWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo);
    void RenderWestWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo);

private:
    float EyeHeight = 0.5f;

    float MapScale = 1;

    //float WallColors[4] = { 1, 0.4f, 0.75f , 0.8f };
    float WallColors[4] = { 1, 1, 1 , 1 };
    float DefaultExteriorZoneLevel = 0.95f;
    float DefaultIntereorZoneLevel = 0.8f;

    Shader WorldShader;

};

