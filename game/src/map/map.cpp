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
    auto state = GetCell(x, y).State;
    return state == MapCellState::Wall || state == MapCellState::Invalid;
}

bool Map::IsCellCapped(int x, int y)
{
    auto cell = GetCell(x, y);
    return (cell.State == MapCellState::Wall || cell.State == MapCellState::Invalid) || (cell.State == MapCellState::Empty && cell.Tiles[1] != MapCellInvalidTile);
}

void Map::Clear()
{
    Cells.clear();
    Size.X = Size.Y = 0;
}