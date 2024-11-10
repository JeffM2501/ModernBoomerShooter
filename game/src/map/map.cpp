#include "map/map.h"

MapCell InvalidCell = { MapCellState::Invalid };

MapCell Map::GetCell(int x, int y)
{
    if (x < 0 || x>= Size.X)
        return InvalidCell;

    if (y < 0 || y >= Size.Y)
        return InvalidCell;

    return Cells[y * Size.X + x];
}

bool Map::IsCellSolid(int x, int y)
{
    return GetCell(x, y).State == MapCellState::Wall;
}

void Map::Clear()
{
    Cells.clear();
    Size.X = Size.Y = 0;
}