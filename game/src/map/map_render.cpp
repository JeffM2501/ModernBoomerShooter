#include "map/map_render.h"
#include "map/raycaster.h"

#include "services/texture_manager.h"
#include "services/global_vars.h"

#include "raymath.h"
#include "rlgl.h"

MapRenderer::MapRenderer(Map& map, Raycaster& caster)
    : WorldMap(map)
    , WorldRaycaster(caster)
{
}

void MapRenderer::SetEyeHeight(float height)
{
    EyeHeight = height;
}

void rlVertex3if(int x, int y, float z)
{
    rlVertex3f(float(x), float(y), z);
}

Color ScaleColor3(Color tint, float factor)
{
    return Color{ uint8_t(tint.r * factor), uint8_t(tint.g * factor), uint8_t(tint.b * factor), tint.a };
}

void rlColor4ubScaled(Color tint, int factor)
{
    float scaleFactor = 1 - ((factor / 3.0f) * 0.3f);

    Color scaleColor = ScaleColor3(tint, scaleFactor);
    rlColor4ub(scaleColor.r, scaleColor.g, scaleColor.b, scaleColor.a);
}

int GetVertexAO(bool a, bool b, bool c)
{
    int val = 0;
    if (a)
        val++;
    if (b)
        val++;
    if (c)
        val++;

    return val;
}

static constexpr float DoorThickness = 0.125f;

void MapRenderer::DrawDoorPanelXAlligned(Color floorColor, Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    // Y side face
    rlNormal3f(0, -1, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(MapScale, DoorThickness * -0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(0, DoorThickness * -0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(0, DoorThickness *-0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(MapScale, DoorThickness * -0.5f, 0);

    // Y side face
    rlNormal3f(0, 1, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(0, DoorThickness * 0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(0, DoorThickness * 0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(MapScale, DoorThickness * 0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(MapScale, DoorThickness * 0.5f, 0);

    // X min side face
    rlNormal3f(-1, 0, 0);

    float thinUVWidth = tileUv.width - tileUv.x;
    float thinUVHeight = tileUv.height - tileUv.y;

    float thinUVMin = tileUv.x + ((0.5f - (DoorThickness / 2)) * thinUVWidth);
    float thinUVMax = tileUv.x + ((0.5f + (DoorThickness / 2)) * thinUVWidth);
    
    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.y);
    rlVertex3f(0, DoorThickness * -0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.y);
    rlVertex3f(0, DoorThickness * 0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.height);
    rlVertex3f(0, DoorThickness * 0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.height);
    rlVertex3f(0, DoorThickness * -0.5f, 0);

    // X max side face
    rlNormal3f(1, 0, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.height);
    rlVertex3f(MapScale, DoorThickness * 0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.y);
    rlVertex3f(MapScale, DoorThickness * 0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.y);
    rlVertex3f(MapScale, DoorThickness * -0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.height);
    rlVertex3f(MapScale, DoorThickness * -0.5f, 0);

    // TODO
    // draw the top
    rlNormal3f(0, 0, 1);

    float topVOffset = thinUVHeight * DoorThickness;
    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height - topVOffset);
    rlVertex3f(0, DoorThickness * -0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height - topVOffset);
    rlVertex3f(MapScale, DoorThickness * -0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(MapScale, DoorThickness * 0.5f, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(0, DoorThickness * 0.5f, MapScale);


    // draw the bottom....
    rlNormal3f(0, 0, -1);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(0, DoorThickness * 0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(MapScale, DoorThickness * 0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height - topVOffset);
    rlVertex3f(MapScale, DoorThickness * -0.5f, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height - topVOffset);
    rlVertex3f(0, DoorThickness * -0.5f, 0);
}


void MapRenderer::DrawDoorPanelYAlligned(Color floorColor, Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float thinUVWidth = tileUv.width - tileUv.x;
    float thinUVHeight = tileUv.height - tileUv.y;

    float thinUVMin = tileUv.x + ((0.5f - (DoorThickness / 2)) * thinUVWidth);
    float thinUVMax = tileUv.x + ((0.5f + (DoorThickness / 2)) * thinUVWidth);

    // Y side face
    rlNormal3f(0, -1, 0);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.y);
    rlVertex3f(DoorThickness * 0.5f, 0, MapScale);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.y);
    rlVertex3f(DoorThickness * -0.5f, 0, MapScale);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.height);
    rlVertex3f(DoorThickness * -0.5f, 0, 0);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.height);
    rlVertex3f(DoorThickness * 0.5f, 0, 0);
 
    // Y side face
    rlNormal3f(0, 1, 0);
    
    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.height);
    rlVertex3f(DoorThickness * -0.5f, MapScale, 0);
    
    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(thinUVMax, tileUv.y);
    rlVertex3f(DoorThickness * -0.5f, MapScale, MapScale);
    
    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.y);
    rlVertex3f(DoorThickness * 0.5f, MapScale, MapScale);
    
    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(thinUVMin, tileUv.height);
    rlVertex3f(DoorThickness * 0.5f, MapScale, 0);

    // X min side face
    rlNormal3f(-1, 0, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(-DoorThickness * 0.5f, 0, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(-DoorThickness * 0.5f, MapScale, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(-DoorThickness * 0.5f, MapScale, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(-DoorThickness * 0.5f, 0, 0);

    // X max side face
    rlNormal3f(1, 0, 0);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(DoorThickness * 0.5f, MapScale, 0);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(DoorThickness * 0.5f, MapScale, MapScale);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(DoorThickness * 0.5f, 0, MapScale);
 
    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(DoorThickness * 0.5f, 0, 0);

    // TODO
    // draw the top
    rlNormal3f(0, 0, 1);

    float topVOffset = thinUVHeight * DoorThickness;
    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height - topVOffset);
    rlVertex3f(DoorThickness * -0.5f, 0, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height - topVOffset);
    rlVertex3f(DoorThickness * 0.5f, 0, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(DoorThickness * 0.5f, MapScale, MapScale);

    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(DoorThickness * -0.5f, MapScale, MapScale);

    // draw the bottom....
    rlNormal3f(0, 0, -1);
    rlColor4ubScaled(floorColor, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(DoorThickness * -0.5f, MapScale, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(DoorThickness * 0.5f, MapScale, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height - topVOffset);
    rlVertex3f(DoorThickness * 0.5f, 0, 0);

    rlColor4ubScaled(floorColor, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height - topVOffset);
    rlVertex3f(DoorThickness * -0.5f, 0, 0);
}

void MapRenderer::RenderDoor(int x, int y, Color floorColor, const AmbientOcclusionCellValues& aoInfo)
{
    MapCell cell = WorldMap.GetCell(x, y);

    bool xAlligned = cell.Flags & MapCellFlags::XAllignment;
    bool openVertical = cell.Flags & MapCellFlags::HorizontalVertical;
    bool backwards = cell.Flags & MapCellFlags::Reversed;

    float halfGrid = MapScale * 0.5f;

    float animParam = cell.ParamState / 257.0f;

    Vector3 animaionOffset = { 0, 0, 0 };

    if (cell.ParamState > 0)
    {
        if (openVertical)
        {
            if (backwards)
                animaionOffset.z = -(animParam * MapScale);
            else
                animaionOffset.z = animParam * MapScale;
        }
        else
        {
            if (xAlligned)
            {
                if (backwards)
                    animaionOffset.x = -(animParam * MapScale);
                else
                    animaionOffset.x = animParam * MapScale;
            }
            else
            {
                if (backwards)
                    animaionOffset.y = -animParam * MapScale;
                else
                    animaionOffset.y = animParam * MapScale;
            }
        }
    }

    if (xAlligned)
    {
        rlPushMatrix();
        rlTranslatef(x * MapScale, y * MapScale, 0);
        rlTranslatef(animaionOffset.x, halfGrid + animaionOffset.y, animaionOffset.z);
        DrawDoorPanelXAlligned(floorColor, WorldMap.TileSourceRects[cell.Tiles[2]], aoInfo);
        rlPopMatrix();
    }
    else
    {
        rlPushMatrix();
        rlTranslatef(x * MapScale, y * MapScale, 0);
        rlTranslatef(animaionOffset.x + halfGrid, animaionOffset.y, animaionOffset.z);
        DrawDoorPanelYAlligned(floorColor, WorldMap.TileSourceRects[cell.Tiles[2]], aoInfo);
        rlPopMatrix();
    }
}

void MapRenderer::RenderNorthWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    rlColor4ub(tint.r, tint.g, tint.b, 255);
    rlNormal3f(0, -1, 0);

    rlColor4ubScaled(tint, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(xMax, yMax, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(xMin, yMax, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(xMin, yMax, 0);

    rlColor4ubScaled(tint, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(xMax, yMax, 0);
}

void MapRenderer::RenderSouthWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    rlColor4ub(tint.r, tint.g, tint.b, 255);
    rlNormal3f(0, 1, 0);

    rlColor4ubScaled(tint, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(xMin, yMin, 0);

    rlColor4ubScaled(tint, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(xMin, yMin, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(xMax, yMin, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(xMax, yMin, 0);
}

void MapRenderer::RenderEastWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    rlColor4ub(tint.r, tint.g, tint.b, 255);
    rlNormal3f(-1, 0, 0);

    rlColor4ubScaled(tint, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(xMax, yMin, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(xMax, yMax, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(xMax, yMax, 0);

    rlColor4ubScaled(tint, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(xMax, yMin, 0);
}

void MapRenderer::RenderWestWall(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    rlColor4ub(tint.r, tint.g, tint.b, 255);
    rlNormal3f(1, 0, 0);

    rlColor4ubScaled(tint, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(xMin, yMax, 0);

    rlColor4ubScaled(tint, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(xMin, yMax, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(xMin, yMin, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(xMin, yMin, 0);
}

void MapRenderer::RenderCeiling(int x, int y, Color tint, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    rlColor4ub(tint.r, tint.g, tint.b, 255);
    rlNormal3f(0, 0, -1);

    rlColor4ubScaled(tint, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(xMin, yMax, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(xMax, yMax, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(xMax, yMin, MapScale);

    rlColor4ubScaled(tint, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(xMin, yMin, MapScale);
}

void MapRenderer::RenderFloor(int x, int y, Color floorColor, Color exteriorFloorColor, const Rectangle& tileUv, const AmbientOcclusionCellValues& aoInfo)
{
    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    Color tint = floorColor;
    if (aoInfo.Values[3].ConveredValue == 0)
        tint = exteriorFloorColor;

    rlNormal3f(0, 0, 1);
    rlColor4ubScaled(tint, aoInfo.Values[3].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.height);
    rlVertex3f(xMin, yMin, 0);

    tint = floorColor;
    if (aoInfo.Values[2].ConveredValue == 0)
        tint = exteriorFloorColor;

    rlColor4ubScaled(tint, aoInfo.Values[2].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.height);
    rlVertex3f(xMax, yMin, 0);

    tint = floorColor;
    if (aoInfo.Values[1].ConveredValue == 0)
        tint = exteriorFloorColor;

    rlColor4ubScaled(tint, aoInfo.Values[1].AOValue);
    rlTexCoord2f(tileUv.width, tileUv.y);
    rlVertex3f(xMax, yMax, 0);

    tint = floorColor;
    if (aoInfo.Values[0].ConveredValue == 0)
        tint = exteriorFloorColor;

    rlColor4ubScaled(tint, aoInfo.Values[0].AOValue);
    rlTexCoord2f(tileUv.x, tileUv.y);
    rlVertex3f(xMin, yMax, 0);
}

void MapRenderer::RenderCell(int x, int y)
{
    MapCell cell = WorldMap.GetCell(x, y);

    if (cell.State == MapCellState::Wall)
        return;

    Color tint = WHITE;

    Rectangle tileUv = { 0,0,0,0 };
    if (cell.Tiles[0] != MapCellInvalidTile)
        tileUv = WorldMap.TileSourceRects[cell.Tiles[0]];

    // todo, only render cells that are open, check edges for walls so we can tint the walls based on the light zone.

    float xMin = x * MapScale;
    float yMin = y * MapScale;

    float xMax = xMin + MapScale;
    float yMax = yMin + MapScale;

    /*  AO Grid
        Y
        | 0-----1
        | |     |
        | |     |
        | 3-----2
        |
        +--------X
    */

    AmbientOcclusionCellValues aoInfo;

    aoInfo.Values[0].AOValue = GetVertexAO(WorldMap.IsCellSolid(x, y + 1), WorldMap.IsCellSolid(x - 1, y + 1), WorldMap.IsCellSolid(x - 1, y));
    aoInfo.Values[0].ConveredValue = GetVertexAO(WorldMap.IsCellCapped(x, y + 1), WorldMap.IsCellCapped(x - 1, y + 1), WorldMap.IsCellCapped(x - 1, y));

    aoInfo.Values[1].AOValue = GetVertexAO(WorldMap.IsCellSolid(x + 1, y), WorldMap.IsCellSolid(x + 1, y + 1), WorldMap.IsCellSolid(x, y + 1));
    aoInfo.Values[1].ConveredValue = GetVertexAO(WorldMap.IsCellCapped(x + 1, y), WorldMap.IsCellCapped(x + 1, y + 1), WorldMap.IsCellCapped(x, y + 1));

    aoInfo.Values[2].AOValue = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x + 1, y - 1), WorldMap.IsCellSolid(x + 1, y));
    aoInfo.Values[2].ConveredValue = GetVertexAO(WorldMap.IsCellCapped(x, y - 1), WorldMap.IsCellCapped(x + 1, y - 1), WorldMap.IsCellCapped(x + 1, y));
   
    aoInfo.Values[3].AOValue = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x - 1, y - 1), WorldMap.IsCellSolid(x - 1, y));
    aoInfo.Values[3].ConveredValue = GetVertexAO(WorldMap.IsCellCapped(x, y - 1), WorldMap.IsCellCapped(x - 1, y - 1), WorldMap.IsCellCapped(x - 1, y));

    // resolve zone color
    float baseColor = DefaultIntereorZoneLevel;
    
    if (cell.LightZone != MapCellInvalidLightZone)
    {
        baseColor = WorldMap.LightZones[cell.LightZone].CurrenSequenceValue;
    }
    else
    {
        if (!WorldMap.IsCellCapped(x, y))
            baseColor = DefaultExteriorZoneLevel;
        else
            baseColor = DefaultIntereorZoneLevel;
    }

    Color floorColor = ScaleColor3(WHITE, baseColor);

    // floor
    if (cell.Tiles[0] != MapCellInvalidTile)
    {
        RenderFloor(x, y, floorColor, ScaleColor3(WHITE, DefaultExteriorZoneLevel), tileUv, aoInfo);
    }

    // ceiling
    if (cell.Tiles[1] != MapCellInvalidTile)
    {
        RenderCeiling(x, y, ScaleColor3(WHITE, baseColor * 0.75f), WorldMap.TileSourceRects[cell.Tiles[1]], aoInfo);
    }

    if (WorldMap.IsCellSolid(x, y + 1))
    {
        RenderNorthWall(x, y, ScaleColor3(floorColor, WallColors[0]), WorldMap.TileSourceRects[WorldMap.GetCell(x, y + 1).Tiles[0]], aoInfo);
    }

    if (WorldMap.IsCellSolid(x, y - 1))
    {
        RenderSouthWall(x, y, ScaleColor3(floorColor, WallColors[1]), WorldMap.TileSourceRects[WorldMap.GetCell(x, y - 1).Tiles[0]], aoInfo);   
    }

    if (WorldMap.IsCellSolid(x + 1, y))
    {
        RenderEastWall(x, y, ScaleColor3(floorColor, WallColors[2]), WorldMap.TileSourceRects[WorldMap.GetCell(x + 1, y).Tiles[0]], aoInfo);
    }

    if (WorldMap.IsCellSolid(x - 1, y))
    {
        RenderWestWall(x, y, ScaleColor3(floorColor, WallColors[3]), WorldMap.TileSourceRects[WorldMap.GetCell(x - 1, y).Tiles[0]], aoInfo);  
    }

    if (cell.State == MapCellState::Door)
    {
        RenderDoor(x, y, floorColor, aoInfo);
    }
}

void MapRenderer::Reset()
{
    Viepoint.fovy = 45;
    Viepoint.up = Vector3UnitZ;
    SetViewpoint(Vector3Zeros, 0, 0);

    WorldShader = TextureManager::GetShader("world");

    Vector2 lightVec = { cosf(DEG2RAD * WorldMap.LightInfo.AmbientAngle), sinf(DEG2RAD * WorldMap.LightInfo.AmbientAngle) };

    if (lightVec.x > 0)
    {
        WallColors[3] = lightVec.x;
        WallColors[2] = lightVec.x * 0.75f;
    }
    else
    {
        WallColors[2] = fabsf(lightVec.x);
        WallColors[3] = fabsf(lightVec.x) * 0.75f;
    }

    if (lightVec.y > 0)
    {
        WallColors[1] = lightVec.y;
        WallColors[0] = lightVec.y * 0.75f;
    }
    else
    {
        WallColors[0] = fabsf(lightVec.y);
        WallColors[1] = fabsf(lightVec.y) * 0.75f;
    }

    DefaultExteriorZoneLevel = WorldMap.LightInfo.ExteriorAmbientLevel;
    DefaultIntereorZoneLevel = WorldMap.LightInfo.InteriorAmbientLevel;
}

void MapRenderer::Render()
{
    BeginMode3D(Viepoint);

    // draw the world bounds
    DrawLine3D(Vector3Zeros, Vector3{ WorldMap.Size.X * MapScale, 0, 0 }, BLUE);
    DrawLine3D(Vector3Zeros, Vector3{ 0, WorldMap.Size.Y * MapScale, 0 }, BLUE);
    DrawLine3D(Vector3{ WorldMap.Size.X * MapScale, 0, 0 }, Vector3{ WorldMap.Size.X * MapScale, WorldMap.Size.Y * MapScale, 0 }, BLUE);
    DrawLine3D(Vector3{ 0, WorldMap.Size.Y * MapScale, 0 }, Vector3{ WorldMap.Size.X * MapScale, WorldMap.Size.Y * MapScale, 0 }, BLUE);

    // draw some axis lines
    float baseArrowRadius = 0.125f / 4.0f;
    DrawCylinder(Vector3Zeros, baseArrowRadius, baseArrowRadius, 1, 12, GREEN);
    DrawCylinder(Vector3UnitY , 0, baseArrowRadius*2, 0.25f, 12, GREEN);

    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    DrawCylinder(Vector3Zeros, baseArrowRadius, baseArrowRadius, 1, 12, BLUE);
    DrawCylinder(Vector3UnitY, 0, baseArrowRadius * 2, 0.25f, 12, BLUE);
    rlPopMatrix();

    rlPushMatrix();
    rlRotatef(90, 0, 0, -1);
    DrawCylinder(Vector3Zeros, baseArrowRadius, baseArrowRadius, 1, 12, RED);
    DrawCylinder(Vector3UnitY, 0, baseArrowRadius * 2, 0.25f, 12, RED);
    rlPopMatrix();

    DrawSphere(Vector3Zeros, baseArrowRadius * 1.5f, WHITE);

    BeginShaderMode(WorldShader);
    {
        rlSetTexture(WorldMap.Tilemap.id);
        rlBegin(RL_QUADS);

        if (GlobalVars::UseVisCulling)
        {
            for (auto cell : WorldRaycaster.GetHitCelList())
            {
                RenderCell(cell.X, cell.Y);
            }
        }
        else
        {
            for (int y = 0; y < WorldMap.Size.Y; y++)
            {
                for (int x = 0; x < WorldMap.Size.X; x++)
                {
                    RenderCell(x, y);
                }
            }
        }
        rlEnd();

        EndShaderMode();
    }

    EndMode3D();
}

void MapRenderer::SetViewpoint(Vector3 position, float yaw, float pitch)
{
    Vector3 forward = Vector3RotateByAxisAngle(Vector3UnitY, Vector3UnitX, -pitch * DEG2RAD);
    forward = Vector3RotateByAxisAngle(forward, Vector3UnitZ, yaw * DEG2RAD);
    Viepoint.position = (position * MapScale) + (Vector3UnitZ * EyeHeight);
    Viepoint.target = Viepoint.position + forward;
}
