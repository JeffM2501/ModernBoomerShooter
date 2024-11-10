#pragma once

#include <stdint.h>
#include <vector>

#include "raylib.h"

struct MapCoordinate
{
    uint16_t X = 0;
    uint16_t Y = 0;

    inline uint32_t GetHash() const { return ((X << 16) | ((Y) & 0xffff)); }
    static uint32_t GetHash(uint16_t x, uint16_t y) { return ((x << 16) | ((y) & 0xffff)); }
};

enum class MapCellState : uint8_t
{
    Empty = 0,
    Wall = 1,
    Door = 2,
    Invalid = 0xFF
};

static uint8_t MapCellInvalidTile = 0xff;

struct MapCell  //32 bits
{
    MapCellState State = MapCellState::Empty;
    uint8_t Tiles[2] = { MapCellInvalidTile, MapCellInvalidTile };
    uint8_t Flags = 0;
};

struct Map
{
    std::vector<MapCell> Cells;
    MapCoordinate Size;
    Texture Tilemap = { 0 };
    std::vector<Rectangle> TileSourceRects;

    MapCell GetCell(int x, int y);
    bool IsCellSolid(int x, int y);
    void Clear();
};