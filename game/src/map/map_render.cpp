#include "map/map_render.h"
#include "raymath.h"
#include "rlgl.h"

MapRenderer::MapRenderer(Map& map)
    : WorldMap(map)
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
    float scaleFactor = 1 - ((factor / 3.0f) * 0.5f);

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

void MapRenderer::RenderCell2(int x, int y)
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
    int AO2 = GetVertexAO(WorldMap.IsCellSolid(x + 1, y), WorldMap.IsCellSolid(x + 1, y + 1), WorldMap.IsCellSolid(x, y + 1));
    int AO3 = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x + 1, y - 1), WorldMap.IsCellSolid(x + 1, y));
    int AO4 = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x - 1, y - 1), WorldMap.IsCellSolid(x - 1, y));

    // floor
    if (cell.Tiles[0] != MapCellInvalidTile)
    {
        tint = IntereorColor;
        if (cell.Tiles[1] == MapCellInvalidTile)
            tint = ExtereorColor;

        rlColor4ub(tint.r, tint.g, tint.b, 255);
        rlNormal3f(0, 0, 1);

        rlColor4ubScaled(tint, AO4);
        rlTexCoord2f(tileUv.x, tileUv.height);
        rlVertex3f(xMin, yMin, 0);

        rlColor4ubScaled(tint, AO3);
        rlTexCoord2f(tileUv.width, tileUv.height);
        rlVertex3f(xMax, yMin, 0);

        rlColor4ubScaled(tint, AO2);
        rlTexCoord2f(tileUv.width, tileUv.y);
        rlVertex3f(xMax, yMax, 0);

        rlColor4ubScaled(tint, AO1);
        rlTexCoord2f(tileUv.x, tileUv.y);
        rlVertex3f(xMin, yMax, 0);
    }

    // ceiling
    if (cell.Tiles[1] != MapCellInvalidTile)
    {
        tileUv = WorldMap.TileSourceRects[cell.Tiles[1]];

        tint = ScaleColor3(IntereorColor, 0.75f);
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

        tint = WallColors[0];

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

        tint = WallColors[1];
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
}

void MapRenderer::RenderCell(int x, int y)
{
    MapCell cell = WorldMap.GetCell(x, y);

    rlSetTexture(WorldMap.Tilemap.id);
    rlBegin(RL_QUADS);
    Rectangle tileUv = { 0,0,0,0 };
    if (cell.Tiles[0] != MapCellInvalidTile)
        tileUv = WorldMap.TileSourceRects[cell.Tiles[0]];

    Color tint = WHITE;

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
    int AO1 = GetVertexAO(WorldMap.IsCellSolid(x, y + 1), WorldMap.IsCellSolid(x -1, y + 1), WorldMap.IsCellSolid(x - 1, y));
    int AO2 = GetVertexAO(WorldMap.IsCellSolid(x + 1, y), WorldMap.IsCellSolid(x + 1, y + 1), WorldMap.IsCellSolid(x, y + 1));
    int AO3 = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x + 1, y - 1), WorldMap.IsCellSolid(x + 1, y));
    int AO4 = GetVertexAO(WorldMap.IsCellSolid(x, y - 1), WorldMap.IsCellSolid(x - 1, y - 1), WorldMap.IsCellSolid(x - 1, y));

    if (cell.State == MapCellState::Wall && cell.Tiles[0] != MapCellInvalidTile)
    { 
        // north 1-2 edge
        if (!WorldMap.IsCellSolid(x, y + 1))
        {
            //  FaceCount++;
             
            tint = WallColors[0];

            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, 1, 0);

            rlColor4ubScaled(tint, AO2);
            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xMax, yMax, 0);

            rlColor4ubScaled(tint, AO1);
            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xMin, yMax, 0);

            rlColor4ubScaled(tint, AO1);
            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xMin, yMax, MapScale);

            rlColor4ubScaled(tint, AO2);
            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xMax, yMax, MapScale);
        }

        // south 3-4 edge
        if (!WorldMap.IsCellSolid(x, y - 1))
        {
            //  FaceCount++;

            tint = WallColors[1];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, -1, 0);

            rlColor4ubScaled(tint, AO3);
            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xMax, yMin, 0);

            rlColor4ubScaled(tint, AO3);
            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xMax, yMin, MapScale);

            rlColor4ubScaled(tint, AO4);
            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xMin, yMin, MapScale);

            rlColor4ubScaled(tint, AO4);
            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xMin, yMin, 0);
        }

        // east 2-3 edge
        if (!WorldMap.IsCellSolid(x + 1, y))
        {
            //  FaceCount++;
            tint = WallColors[2];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(1, 0, 0);

            rlColor4ubScaled(tint, AO3);
            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xMax, yMin, 0);

            rlColor4ubScaled(tint, AO2);
            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xMax, yMax, 0);

            rlColor4ubScaled(tint, AO2);
            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xMax, yMax, MapScale);

            rlColor4ubScaled(tint, AO3);
            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xMax, yMin, MapScale);
        }

        // west 1-4 edge
        if (!WorldMap.IsCellSolid(x - 1, y))
        {
            // FaceCount++;
            tint = WallColors[3];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(-1, 0, 0);

            rlColor4ubScaled(tint, AO4);
            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xMin, yMin, 0);

            rlColor4ubScaled(tint, AO4);
            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xMin, yMin, MapScale);

            rlColor4ubScaled(tint, AO1);
            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xMin, yMax, MapScale);

            rlColor4ubScaled(tint, AO1);
            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xMin, yMax, 0);
        }
    }
    else if (cell.State == MapCellState::Empty)
    {
        // floor
        if (cell.Tiles[0] != MapCellInvalidTile)
        {
            tint = IntereorColor;
            if (cell.Tiles[1] == MapCellInvalidTile)
                tint = ExtereorColor;

            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, 0, 1);

            rlColor4ubScaled(tint, AO4);
            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xMin, yMin, 0);

            rlColor4ubScaled(tint, AO3);
            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xMax, yMin, 0);

            rlColor4ubScaled(tint, AO2);
            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xMax, yMax, 0);

            rlColor4ubScaled(tint, AO1);
            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xMin, yMax, 0);
        }

        // ceiling
        if (cell.Tiles[1] != MapCellInvalidTile)
        {
            tileUv = WorldMap.TileSourceRects[cell.Tiles[1]];

            tint = ScaleColor3(IntereorColor, 0.75f);
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
    }

    rlEnd();
}

void MapRenderer::Reset()
{
    Viepoint.fovy = 45;
    Viepoint.up = Vector3UnitZ;
    SetViewpoint(Vector3Zeros, 0, 0);

    Vector2 lightVec = { cosf(DEG2RAD * WorldMap.LightInfo.AmbientAngle), cosf(DEG2RAD * WorldMap.LightInfo.AmbientAngle) };

    if (lightVec.x > 0)
    {
        WallColors[3] = ScaleColor3(WHITE, lightVec.x);
        WallColors[2] = ScaleColor3(WHITE, lightVec.x * 0.75f);
    }
    else
    {
        WallColors[2] = ScaleColor3(WHITE, fabsf(lightVec.x));
        WallColors[3] = ScaleColor3(WHITE, fabsf(lightVec.x) * 0.75f);
    }

    if (lightVec.y > 0)
    {
        WallColors[1] = ScaleColor3(WHITE, lightVec.y);
        WallColors[0] = ScaleColor3(WHITE, lightVec.y * 0.75f);
    }
    else
    {
        WallColors[0] = ScaleColor3(WHITE, fabsf(lightVec.y));
        WallColors[1] = ScaleColor3(WHITE, fabsf(lightVec.y) * 0.75f);
    }

    ExtereorColor = ScaleColor3(WHITE, WorldMap.LightInfo.ExteriorAmbientLevel);
    IntereorColor = ScaleColor3(WHITE, WorldMap.LightInfo.InteriorAmbientLevel);
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

    // todo, get the PVS
    bool onlyEmpty = false;
    
    if (onlyEmpty)
    {
        rlSetTexture(WorldMap.Tilemap.id);
        rlBegin(RL_QUADS);
        for (int y = 0; y < WorldMap.Size.Y; y++)
        {
            for (int x = 0; x < WorldMap.Size.X; x++)
            {
                RenderCell2(x, y);
            }
        }
        rlEnd();
        
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

    EndMode3D();
}

void MapRenderer::SetViewpoint(Vector3 position, float yaw, float pitch)
{
    Vector3 forward = Vector3RotateByAxisAngle(Vector3UnitY, Vector3UnitX, -pitch * DEG2RAD);
    forward = Vector3RotateByAxisAngle(forward, Vector3UnitZ, yaw * DEG2RAD);
    Viepoint.position = (position * MapScale) + (Vector3UnitZ * EyeHeight);
    Viepoint.target = Viepoint.position + forward;
}
