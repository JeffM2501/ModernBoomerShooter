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

void MapRenderer::RenderCell(int x, int y)
{
    MapCell cell = WorldMap.GetCell(x, y);

    rlSetTexture(WorldMap.Tilemap.id);
    rlBegin(RL_QUADS);
    Color tint = WHITE;

    Rectangle tileUv = { 0,0,0,0 };
    if (cell.Tiles[0] != MapCellInvalidTile)
        tileUv = WorldMap.TileSourceRects[cell.Tiles[0]];

    static Color wallColors[6] = { WHITE, Color{128,128,128,255}, Color{196,196,196,255} , Color{200,200,200,255}, GRAY, WHITE };

    float xPos = x * MapScale;
    float yPos = y * MapScale;

    if (cell.State == MapCellState::Wall && cell.Tiles[0] != MapCellInvalidTile)
    {
        if (!WorldMap.IsCellSolid(x, y + 1))
        {
            //  FaceCount++;
              // north
            tint = wallColors[0];

            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, 1, 0);

            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xPos + MapScale, yPos + MapScale, 0);

            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xPos, yPos + MapScale, 0);

            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xPos, yPos + MapScale, MapScale);

            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xPos + MapScale, yPos + MapScale, MapScale);
        }

        // south
        if (!WorldMap.IsCellSolid(x, y - 1))
        {
            //  FaceCount++;

            tint = wallColors[1];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, -1, 0);

            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xPos + MapScale, yPos, 0);

            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xPos + MapScale, yPos, MapScale);

            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xPos, yPos, MapScale);

            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xPos, yPos, 0);
        }

        // east
        if (!WorldMap.IsCellSolid(x + 1, y))
        {
            //  FaceCount++;
            tint = wallColors[2];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(1, 0, 0);

            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xPos + MapScale, yPos, 0);

            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xPos + MapScale, yPos + MapScale, 0);

            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xPos + MapScale, yPos + MapScale, MapScale);

            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xPos + MapScale, yPos, MapScale);
        }

        // west
        if (!WorldMap.IsCellSolid(x - 1, y))
        {
            // FaceCount++;
            tint = wallColors[3];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(-1, 0, 0);

            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xPos, yPos, 0);

            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xPos, yPos, MapScale);

            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xPos, yPos + MapScale, MapScale);

            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xPos, yPos + MapScale, 0);
        }
    }
    else if (cell.State == MapCellState::Empty)
    {
        // floor
        if (cell.Tiles[0] != MapCellInvalidTile)
        {
            tint = wallColors[4];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, 0, 1);

            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xPos, yPos, 0);

            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xPos + MapScale, yPos, 0);

            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xPos + MapScale, yPos + MapScale, 0);

            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xPos, yPos + MapScale, 0);
        }

        // ceiling
        if (cell.Tiles[1] != MapCellInvalidTile)
        {
            tileUv = WorldMap.TileSourceRects[cell.Tiles[1]];

            tint = wallColors[5];
            rlColor4ub(tint.r, tint.g, tint.b, 255);
            rlNormal3f(0, 0, -1);

            rlTexCoord2f(tileUv.x, tileUv.y);
            rlVertex3f(xPos, yPos + MapScale, MapScale);

            rlTexCoord2f(tileUv.width, tileUv.y);
            rlVertex3f(xPos + MapScale, yPos + MapScale, MapScale);

            rlTexCoord2f(tileUv.width, tileUv.height);
            rlVertex3f(xPos + MapScale, yPos, MapScale);

            rlTexCoord2f(tileUv.x, tileUv.height);
            rlVertex3f(xPos, yPos, MapScale);
        }
    }

    rlEnd();
}

void MapRenderer::Reset()
{
    Viepoint.fovy = 45;
    Viepoint.up = Vector3UnitZ;
    SetViewpoint(Vector3Zeros, 0, 0);
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
    for (int y = 0; y < WorldMap.Size.Y; y++)
    {
        for (int x = 0; x < WorldMap.Size.X; x++)
        {
            RenderCell(x, y);
        }
    }

    EndMode3D();
}

void MapRenderer::SetViewpoint(Vector3 position, float yaw, float pitch)
{
    pitch = 0;

    Vector3 forward = Vector3RotateByAxisAngle(Vector3UnitY, Vector3UnitX, pitch * DEG2RAD);
    forward = Vector3RotateByAxisAngle(forward, Vector3UnitZ, yaw * DEG2RAD);
    Viepoint.position = position + (Vector3UnitZ * EyeHeight);
    Viepoint.target = Viepoint.position + forward;
}
