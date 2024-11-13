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