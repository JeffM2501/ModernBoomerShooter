#include "map/raycaster.h"


void Raycaster::SetOutputSize(int renderWidth, float renderFOV)
{
    RenderWidth = renderWidth;

    NominalCameraPlane.y = -tanf(renderFOV * DEG2RAD * 0.5f);
    NominalCameraPlane.x = 0;

    RaySet.resize(renderWidth);
}

void Raycaster::SetMap(const Map* map)
{
    if (map)
    {
        WorldMap = map;
        CellStatus.resize(WorldMap->Size.X * WorldMap->Size.Y);
        CellStatus.assign(CellStatus.size(), 0);
    }
}

void Raycaster::StartFrame(const Vector3& viewLocation, const Vector3& facingVector)
{
    // set the camera plane for this view
    float angle = atan2f(facingVector.y, facingVector.x);
    CameraPlane = Vector2Rotate(NominalCameraPlane, angle);

    // clear any previous hit cells
    for (const auto& i : HitCells)
        CellStatus[i] = 0;

    HitCells.clear();
    HitCellLocs.clear();

    CastCount = 0;

    // the 9 cells around us are always visible
    int x = int(floorf(viewLocation.x));
    int y = int(floorf(viewLocation.y));

    SetCellVis(x, y);

    SetCellVis(x + 1, y + 1);
    SetCellVis(x + 1, y);
    SetCellVis(x + 1, y - 1);

    SetCellVis(x - 1, y + 1);
    SetCellVis(x - 1, y);
    SetCellVis(x - 1, y - 1);

    SetCellVis(x, y + 1);
    SetCellVis(x, y - 1);

    // cast this frame
    UpdateRayset(viewLocation, facingVector);
}

// cast a ray and find out what it hits
void Raycaster::CastRay(RayResult& ray, const Vector3& pos)
{
    ray.Distance = -1;
    if (!WorldMap)
        return;

    CastCount++;

    // The current grid point we are in
    int mapX = int(floor(pos.x));
    int mapY = int(floor(pos.y));

    ray.HitGridType = WorldMap->GetCell(mapX, mapY).Tiles[0];

    //length of ray from current position to next x or y-side
    float sideDistX = 0;
    float sideDistY = 0;

    // length of ray from one x or y-side to next x or y-side
    // these are derived as:
    // deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
    // deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
    // which can be simplified to abs(|rayDir| / rayDirX) and abs(|rayDir| / rayDirY)
    // where |rayDir| is the length of the vector (rayDirX, rayDirY). Its length,
    // unlike (dirX, dirY) is not 1, however this does not matter, only the
    // ratio between deltaDistX and deltaDistY matters, due to the way the DDA
    // stepping further below works. So the values can be computed as below.
    // Division through zero is prevented, even though technically that's not
    // needed in C++ with IEEE 754 floating point values.
    float deltaDistX = (ray.Directon.x == 0) ? float(1e30) : float(fabs(1.0f / ray.Directon.x));
    float deltaDistY = (ray.Directon.y == 0) ? float(1e30) : float(fabs(1.0f / ray.Directon.y));

    float perpWallDist = 0;

    // what direction to step in x or y-direction (either +1 or -1)
    int stepX = 0;
    int stepY = 0;

    bool hit = false; //was there a wall hit?
    bool side = false; //was a NS or a EW wall hit?

    // calculate step and initial sideDist
    if (ray.Directon.x < 0)
    {
        stepX = -1;
        sideDistX = (pos.x - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0f - pos.x) * deltaDistX;
    }

    if (ray.Directon.y < 0)
    {
        stepY = -1;
        sideDistY = (pos.y - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0f - pos.y) * deltaDistY;
    }

    // perform DDA Digital Differential Analyzer to walk the line
    while (!hit)
    {
        //jump to next map square, either in x-direction, or in y-direction
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = false;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = true;
        }

        if (mapX >= WorldMap->Size.X || mapX < 0 || mapY >= WorldMap->Size.Y || mapY < 0)
            break;

        ray.HitGridType = 0;
        if (WorldMap->IsCellSolid(mapX, mapY))
            ray.HitGridType = WorldMap->GetCell(mapX, mapY).Tiles[0];

        ray.HitCellIndex = int(WorldMap->GetCellIndex(mapX, mapY));
        ray.TargetCell.X = mapX;
        ray.TargetCell.Y = mapY;

        //Check if ray has hit a wall
        if (ray.HitGridType != 0)
            hit = true;

        SetCellVis(mapX, mapY);
    }

    if (!hit)
    {
        ray.Distance = -1;
        return;
    }


    // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
    // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
    // This can be computed as (mapX - posX + (1 - stepX) / 2) / rayDirX for side == 0, or same formula with Y
    // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
    // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
    // steps, but we subtract deltaDist once because one step more into the wall was taken above.
    if (!side)
    {
        perpWallDist = (sideDistX - deltaDistX);
        ray.Normal = stepX < 0 ? HitNormals::East : HitNormals::West;
    }
    else
    {
        perpWallDist = (sideDistY - deltaDistY);
        ray.Normal = stepY < 0 ? HitNormals::North : HitNormals::South;
    }

    ray.Distance = perpWallDist;
}

bool Raycaster::CastRayPair(int minPixel, int maxPixel, const Vector3& viewLocation, const Vector3& facingVector)
{
    float cameraX = 0;

    RayResult& minRay = RaySet[minPixel];
    RayResult& maxRay = RaySet[maxPixel];

    // we've been here before
    if (minRay.HitCellIndex >= 0 && maxRay.HitCellIndex >= 0 && maxPixel - minPixel <= 1)
        return true;

    if (minRay.HitCellIndex < 0)
    {
        cameraX = 2 * minPixel / (float)RenderWidth - 1; //x-coordinate in camera space
        minRay.Directon.x = facingVector.x + CameraPlane.x * cameraX;
        minRay.Directon.y = facingVector.y + CameraPlane.y * cameraX;
        CastRay(minRay, viewLocation);
    }

    if (maxRay.HitCellIndex < 0)
    {
        cameraX = 2 * maxPixel / (float)RenderWidth - 1; //x-coordinate in camera space
        maxRay.Directon.x = facingVector.x + CameraPlane.x * cameraX;
        maxRay.Directon.y = facingVector.y + CameraPlane.y * cameraX;

        CastRay(maxRay, viewLocation);
    }

    if (maxRay.Distance < 0 && minRay.Distance < 0)
        return true;

    return minRay.HitCellIndex == maxRay.HitCellIndex;
}

void Raycaster::UpdateRayset(const Vector3& viewLocation, const Vector3& facingVector)
{
    SetCellVis(int(viewLocation.x), int(viewLocation.y));

    for (uint16_t i = 0; i < RenderWidth; i++)
        RaySet[i].HitCellIndex = -1;

    size_t index = 0;
    std::vector<std::pair<int, int>> pendingCasts;

    pendingCasts.reserve(RenderWidth);
    pendingCasts.emplace_back(0, RenderWidth - 1);

    while (index < pendingCasts.size())
    {
        int min = pendingCasts[index].first;
        int max = pendingCasts[index].second;

        if (!CastRayPair(min, max, viewLocation, facingVector))
        {
            if (max - min > 1)
            {
                int bisector = ((max - min) / 2) + min;

                if (min != bisector)
                    pendingCasts.emplace_back(min, bisector);

                if (max != bisector)
                    pendingCasts.emplace_back(bisector, max);
            }
        }

        index++;
    }
}

bool Raycaster::IsCellVis(int x, int y) const
{
    if (!WorldMap || x < 0 || x >= WorldMap->Size.X || y < 0 || y >= WorldMap->Size.Y)
        return false;

    int index = y * (int)WorldMap->Size.X + x;
    return CellStatus[index] == 1;
}

void Raycaster::AddCellVis(int x, int y)
{
    int index = y * (int)WorldMap->Size.X + x;
    uint8_t& id = CellStatus[index];
    if (id == 1)
        return;

    id = 1;
    HitCells.push_back(index);
    HitCellLocs.emplace_back(MapCoordinate{ uint16_t(x), uint16_t(y) });
}

void Raycaster::SetCellVis(int x, int y)
{
    auto state = WorldMap->GetCell(x, y).State;
    if (WorldMap->IsCellSolid(x, y))
    {
        // if we hit a wall, ensure that every cell around it that is passable is tagged so we see all the walls
        for (int yOffset = -1; yOffset <= 1; yOffset++)
        {
            for (int xOffset = -1; xOffset <= 1; xOffset++)
            {
                if (!WorldMap->IsCellSolid(x + xOffset, y + yOffset))
                {
                    AddCellVis(x + xOffset, y + yOffset);
                }
            }
        }
    }
    else
    {
        AddCellVis(x, y);
    }
}
