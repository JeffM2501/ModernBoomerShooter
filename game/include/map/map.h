#pragma once

#include <stdint.h>
#include <vector>
#include <string>

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

static constexpr uint8_t MapCellInvalidTile = 0xff;
static constexpr uint8_t MapCellInvalidLightZone = 0xff;

struct MapCell  //pad to 64 bits
{
    MapCellState State = MapCellState::Empty;
    uint8_t Tiles[2] = { MapCellInvalidTile, MapCellInvalidTile };
    uint8_t Flags = 0;

    uint8_t LightZone = MapCellInvalidLightZone;
};

struct LightingInfo
{
    std::string SkyboxTextureName;

    float ExteriorAmbientLevel = 1;
    float InteriorAmbientLevel = 0.75f;
    float AmbientAngle = 45;
};

struct LightZoneInfo
{
    float MaxLevel = 1;
    float MinLevel = 0.25f;
    float SequenceLenght = 1;
    std::vector<float> SequenceValues;
    
    size_t CurrentSequenceIndex = 0;
    float CurrentSequenceParam = 0;
    float SequenceFrameTime = 0.1f;

    float CurrenSequenceValue = 1;
    void Advance();

    void Reset();
};

struct Map
{
    std::vector<MapCell> Cells;
    MapCoordinate Size;
    Texture Tilemap = { 0 };
    std::vector<Rectangle> TileSourceRects;

    LightingInfo LightInfo;

    std::vector<LightZoneInfo> LightZones;

    MapCell GetCell(int x, int y) const;
    MapCell &GetCellRef(int x, int y);
    bool IsCellSolid(int x, int y);
    bool IsCellCapped(int x, int y);
    void Clear();
};