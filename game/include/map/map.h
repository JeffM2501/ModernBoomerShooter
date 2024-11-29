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

namespace MapCellFlags
{
    static constexpr uint8_t None = (1u << 0);
    static constexpr uint8_t HorizontalVertical = (1u << 1);
    static constexpr uint8_t XAllignment = (1u << 2);
    static constexpr uint8_t Impassible = (1u << 3);
    static constexpr uint8_t Split = (1u << 4);
    static constexpr uint8_t Reversed = (1u << 5);
}

static constexpr uint8_t MapCellInvalidTile = 0xff;
static constexpr uint8_t MapCellInvalidLightZone = 0xff;

struct MapCell  //pad to 64 bits
{
    MapCellState State = MapCellState::Empty;
    uint8_t Flags = 0;
    uint8_t LightZone = MapCellInvalidLightZone;
    uint8_t ParamState = 0;

    uint8_t Tiles[4] = { MapCellInvalidTile, MapCellInvalidTile, MapCellInvalidTile, MapCellInvalidTile };
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
    MapCell& GetCellRef(int x, int y);
    const MapCell& GetCellRef(int x, int y) const;
    bool IsCellSolid(int x, int y) const;
    bool IsCellPassable(int x, int y) const;
    bool IsCellCapped(int x, int y) const;
    void Clear();

    inline size_t GetCellIndex(int x, int y) const { return y * Size.X + x; }

    bool MoveEntity(Vector3& position, Vector3& desiredMotion, float radius);

    std::vector<size_t> DoorCells;

private:
    void PointNearesGridPoint(int x, int y, const Vector3 point, Vector3* nearest, Vector3* normal);
};