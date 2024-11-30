#include "map/map.h"

#include "services/game_time.h"

#include "raymath.h"

MapCell InvalidCell = { MapCellState::Invalid };

void LightZoneInfo::Advance()
{
    if (SequenceValues.size() < 2)
        return;

    CurrentSequenceParam += GameTime::GetDeltaTime();

    while (CurrentSequenceParam > SequenceFrameTime)
    {
        CurrentSequenceParam -= SequenceFrameTime;
        CurrentSequenceIndex++;
        if (CurrentSequenceIndex >= SequenceValues.size())
            CurrentSequenceIndex = 0;
    }

    float t = CurrentSequenceParam / SequenceFrameTime;

    size_t nextValue = CurrentSequenceIndex + 1;

    if (nextValue >= SequenceValues.size())
        nextValue = 0;

    CurrenSequenceValue = Lerp(SequenceValues[CurrentSequenceIndex], SequenceValues[nextValue], t);

    CurrenSequenceValue = std::min(MaxLevel, MinLevel + ((MaxLevel - MinLevel) * CurrenSequenceValue));
}

void LightZoneInfo::Reset()
{
    CurrenSequenceValue = MaxLevel;
    CurrentSequenceIndex = 0;
    CurrentSequenceParam = 0;
    if (!SequenceValues.empty())
        CurrenSequenceValue = std::min(MaxLevel, MinLevel + ((MaxLevel-MinLevel) * SequenceValues[0]));
}


MapCell Map::GetCell(int x, int y) const
{
    if (x < 0 || x>= Size.X)
        return InvalidCell;

    if (y < 0 || y >= Size.Y)
        return InvalidCell;

    return Cells[y * Size.X + x];
}

MapCell& Map::GetCellRef(int x, int y)
{
    if (x < 0 || x >= Size.X)
        return InvalidCell;

    if (y < 0 || y >= Size.Y)
        return InvalidCell;

    return Cells[y * Size.X + x];
}

const MapCell& Map::GetCellRef(int x, int y) const
{
    if (x < 0 || x >= Size.X)
        return InvalidCell;

    if (y < 0 || y >= Size.Y)
        return InvalidCell;

    return Cells[y * Size.X + x];
}

bool Map::IsCellSolid(int x, int y) const
{
    auto state = GetCell(x, y).State;
    return state == MapCellState::Wall || state == MapCellState::Invalid;
}

bool Map::IsCellPassable(int x, int y) const
{
    MapCell cell = GetCell(x, y);
    bool impassable = cell.Flags & MapCellFlags::Impassible;

    return !impassable && (cell.State == MapCellState::Empty || cell.State == MapCellState::Door);
}

bool Map::IsCellCapped(int x, int y) const
{
    auto cell = GetCell(x, y);
    return (cell.State == MapCellState::Wall || cell.State == MapCellState::Invalid) || (cell.State == MapCellState::Empty && cell.Tiles[1] != MapCellInvalidTile);
}

void Map::Clear()
{
    Cells.clear();
    Size.X = Size.Y = 0;
}

bool Map::MoveEntity(Vector3& position, Vector3& desiredMotion, float radius)
{
    Vector3 newPos = position + desiredMotion;
    bool collided = false;

    for (int y = int(position.y - 1); y <= int(position.y + 1); y++)
    {
        for (int x = int(position.x - 1); x <= int(position.x + 1); x++)
        {
            if (!IsCellPassable(x, y))
            {
                // check rectangle

                Vector3 hitPoint = { -10000,-100000, 0 };
                Vector3 hitNormal = { 0, 0, 0 };
                PointNearesGridPoint(x, y, newPos, &hitPoint, &hitNormal);

                Vector3 vectorToHit = hitPoint - newPos;

                bool inside = Vector3LengthSqr(vectorToHit) < radius * radius;

                if (inside)
                {
                    collided = true;
                    // normalize the vector along the point to where we are nearest
                    vectorToHit = Vector3Normalize(vectorToHit);

                    // project that out to the radius to find the point that should be 'deepest' into the rectangle.
                    Vector3 projectedPoint = newPos + (vectorToHit * radius);

                    // compute the shift to take the deepest point out to the edge of our nearest hit, based on the vector direction
                    Vector3 delta = { 0,0 };

                    if (hitNormal.x != 0)
                        delta.x = hitPoint.x - projectedPoint.x;
                    else
                        delta.y = hitPoint.y - projectedPoint.y;

                    // shift the new point by the delta to push us outside of the rectangle
                    newPos += delta;
                }
            }
        }
    }

    desiredMotion = newPos - position;

    return collided;
}


void Map::PointNearesGridPoint(int x, int y, const Vector3 point, Vector3* nearest, Vector3* normal)
{
    // get the closest point on the vertical sides
    float hValue = float(x);
    float hNormal = -1.0f;
    if (point.x > x + 1)
    {
        hValue += 1;
        hNormal = 1;
    }

    Vector3 vecToPoint = Vector3{ hValue, float(y), 0.0f } -  point;

    // get the dot product between the ray and the vector to the point
    float dotForPoint = Vector3DotProduct(Vector3{ 0, -1, 0 }, vecToPoint);
    Vector3 nearestPoint = { hValue, 0, 0 };

    if (dotForPoint < 0)
        nearestPoint.y = float(y);
    else if (dotForPoint >= 1)
        nearestPoint.y = float(y + 1);
    else
        nearestPoint.y = y + dotForPoint;

    // get the closest point on the horizontal sides
    float vValue = float(y);
    float vNormal = -1;
    if (point.y > y + 1)
    {
        vValue = float(y + 1);
        vNormal = 1;
    }

    vecToPoint = Vector3{ float(x), vValue, 0 } - point;
    // get the dot product between the ray and the vector to the point
    dotForPoint = Vector3DotProduct(Vector3{ -1, 0, 0 }, vecToPoint);
    *nearest = Vector3{ 0,vValue, 0};

    if (dotForPoint < 0)
        nearest->x = float(x);
    else if (dotForPoint >= 1)
        nearest->x = float(x + 1);
    else
        nearest->x = float(x + dotForPoint);

    if (Vector3LengthSqr(point -nearestPoint) < Vector3LengthSqr(point - *nearest))
    {
        *nearest = nearestPoint;

        if (normal)
        {
            normal->x = hNormal;
            normal->y = 0;
        }
    }
    else
    {
        if (normal)
        {
            normal->y = vNormal;
            normal->x = 0;
        }
    }
}