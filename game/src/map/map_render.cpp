#include "map/map_render.h"
#include "map/raycaster.h"

#include "services/texture_manager.h"

#include "raymath.h"
#include "rlgl.h"

MapRenderer::MapRenderer(Map& map, Raycaster& caster)
    : WorldMap(map)
    , WorldRaycaster(caster)
{
    Reset();
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

void MapRenderer::RenderCell(int x, int y)
{
    MapCell cell = WorldMap.GetCell(x, y);

    if (cell.State != MapCellState::Empty && cell.State != MapCellState::Door)
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
        | 1-----2
        | |     |
        | |     |
        | 4-----3
        |
        +--------X
    */
    int AO1 = GetVertexAO(WorldMap.IsCellSolid(x, y + 1), WorldMap.IsCellSolid(x - 1, y + 1), WorldMap.IsCellSolid(x - 1, y));
    int AO1Covered = GetVertexAO(WorldMap.IsCellCapped(x, y + 1), WorldMap.IsCellCapped(x - 1, y + 1), WorldMap.IsCellCapped(x - 1, y));

    int AO2 = GetVertexAO(WorldMap.IsCellSolid(x + 1, y), WorldMap.IsCellSolid(x + 1, y + 1), WorldMap.IsCellSolid(x, y + 1));
    int AO2Covered = GetVertexAO(WorldMap.IsCellCapped(x + 1, y), WorldMap.IsCellCapped(x + 1, y + 1), WorldMap.IsCellCapped(x, y + 1));

    int AO3 = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x + 1, y - 1), WorldMap.IsCellSolid(x + 1, y));
    int AO3Covered = GetVertexAO(WorldMap.IsCellCapped(x, y - 1), WorldMap.IsCellCapped(x + 1, y - 1), WorldMap.IsCellCapped(x + 1, y));
   
    int AO4 = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x - 1, y - 1), WorldMap.IsCellSolid(x - 1, y));
    int AO4Covered = GetVertexAO(WorldMap.IsCellCapped(x, y - 1), WorldMap.IsCellCapped(x - 1, y - 1), WorldMap.IsCellCapped(x - 1, y));

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
        rlNormal3f(0, 0, 1);

        Color exteriorFloorColor = ScaleColor3(WHITE, DefaultExteriorZoneLevel);

        tint = floorColor;
        if (AO4Covered == 0)
            tint = exteriorFloorColor;

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMin, yMin, 0);

        tint = floorColor;
        if (AO3Covered == 0)
            tint = exteriorFloorColor;

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMax, yMin, 0);

        tint = floorColor;
        if (AO2Covered == 0)
            tint = exteriorFloorColor;

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMax, yMax, 0);

        tint = floorColor;
        if (AO1Covered == 0)
            tint = exteriorFloorColor;

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMin, yMax, 0);
    }

    // ceiling
    if (cell.Tiles[1] != MapCellInvalidTile)
    {
        Color ceilingColor = ScaleColor3(WHITE, baseColor * 0.75f);
        tileUv = WorldMap.TileSourceRects[cell.Tiles[1]];

        tint = ceilingColor;
        rlColor4ub(tint.r, tint.g, tint.b, 255);
        rlNormal3f(0, 0, -1);

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMin, yMax, MapScale);

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMax, yMax, MapScale);

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMax, yMin, MapScale);

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMin, yMin, MapScale);
    }

    if (WorldMap.IsCellSolid(x, y + 1))
    {
        tileUv = WorldMap.TileSourceRects[WorldMap.GetCell(x,y+1).Tiles[0]];

        tint = ScaleColor3(floorColor, WallColors[0]);

        rlColor4ub(tint.r, tint.g, tint.b, 255);
        rlNormal3f(0, -1, 0);

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMax, yMax, MapScale);

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMin, yMax, MapScale);

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMin, yMax, 0);

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMax, yMax, 0);
    }

    if (WorldMap.IsCellSolid(x, y - 1))
    {
        tileUv = WorldMap.TileSourceRects[WorldMap.GetCell(x, y - 1).Tiles[0]];

        tint = ScaleColor3(floorColor, WallColors[1]);
        rlColor4ub(tint.r, tint.g, tint.b, 255);
        rlNormal3f(0, 1, 0);

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMin, yMin, 0);

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMin, yMin, MapScale);

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMax, yMin, MapScale);

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMax, yMin, 0);
    }

    if (WorldMap.IsCellSolid(x + 1, y))
    {
        tileUv = WorldMap.TileSourceRects[WorldMap.GetCell(x + 1, y).Tiles[0]];

        //  FaceCount++;
        tint = ScaleColor3(floorColor, WallColors[2]);
        rlColor4ub(tint.r, tint.g, tint.b, 255);
        rlNormal3f(-1, 0, 0);

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMax, yMin, MapScale);

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMax, yMax, MapScale);

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMax, yMax, 0);

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMax, yMin, 0);
    }

    if (WorldMap.IsCellSolid(x - 1, y))
    {
        tileUv = WorldMap.TileSourceRects[WorldMap.GetCell(x - 1, y).Tiles[0]];
        tint = ScaleColor3(floorColor, WallColors[3]);
        rlColor4ub(tint.r, tint.g, tint.b, 255);
        rlNormal3f(1, 0, 0);

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMin, yMax, 0);

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMin, yMax, MapScale);

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMin, yMin, MapScale);

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMin, yMin, 0);
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

    static bool cull = true;

    BeginShaderMode(WorldShader);
    {
        rlSetTexture(WorldMap.Tilemap.id);
        rlBegin(RL_QUADS);

        if (cull)
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
