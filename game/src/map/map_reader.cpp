#include "map/map_reader.h"
#include "world.h"
#include "map/map.h"

#include "services/texture_manager.h"
#include "services/resource_manager.h"

#include "components/spawn_point_component.h"
#include "components/transform_component.h"

#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/FileReader.hpp"

void* ReadXMLDataCallback(const char* filePath, size_t& size)
{
    auto resource = ResourceManager::OpenResource(filePath);

    if (!resource)
        return nullptr;

    size = resource->DataSize;
    return resource->DataBuffer;
}

void ReleaseXMLDataCallback(const char* filePath, void* data)
{
    ResourceManager::ReleaseResource(filePath);
}

using LayerTileCallback = std::function<void(const tmx::TileLayer::Tile&, int, int, size_t)>;

void DoForEachTile(World& world, tmx::TileLayer& layer, LayerTileCallback callback)
{
    if (!callback)
        return;

    auto& tiles = layer.getTiles();

    auto mapSize = world.GetMap().Size;

    int x = 0;
    int y = 0;
    for (size_t i = 0; i < tiles.size(); i++)
    {
        size_t mapCellIndex = (mapSize.Y - y - 1) * mapSize.X + x;
        callback(tiles[i], x, y, mapCellIndex);
        x++;
        if (x >= mapSize.X)
        {
            y++;
            x = 0;
        }
    }
}

bool DoForEachTileInLayer(World& world, tmx::Map& map, std::string_view layerClass, LayerTileCallback callback)
{
    if (!callback)
        return false;

    for (auto& layer : map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Tile || layer->getClass() != layerClass)
            continue;

        DoForEachTile(world, layer->getLayerAs<tmx::TileLayer>(), callback);

        return true;
    }

    return false;
}

using LayerObjectCallback = std::function<void(const tmx::Object&)>;
void DoForEachObject(World& world, tmx::ObjectGroup& group, LayerObjectCallback callback, std::string_view objectClass = "")
{
    if (!callback)
        return;

    for (auto& object : group.getObjects())
    {
        if (objectClass.empty() || object.getClass() == objectClass)
            callback(object);
    }
}

bool DoForEachObjectInLayer(World& world, tmx::Map& map, std::string_view layerClass, LayerObjectCallback callback, std::string_view objectClass = "")
{
    if (!callback)
        return false;

    for (auto& layer : map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Object || layer->getClass() != layerClass)
            continue;

        DoForEachObject(world, layer->getLayerAs<tmx::ObjectGroup>(), callback, objectClass);

        return true;
    }

    return false;
}

bool FindProperty(std::string_view name, const std::vector<tmx::Property>& properties, std::string& stringValue)
{
    for (auto& property : properties)
    {
        if (property.getType() == tmx::Property::Type::String)
            continue;

        if (property.getName() == name)
        {
            stringValue = property.getStringValue();
            return true;
        }
    }

    return false;
}

bool FindProperty(std::string_view name, const std::vector<tmx::Property>& properties, float& floatValue)
{
    for (auto& property : properties)
    {
        if (property.getType() == tmx::Property::Type::Float)
            continue;

        if (property.getName() == name)
        {
            floatValue = property.getFloatValue();
            return true;
        }
    }

    return false;
}

void SetObjectTransform(const tmx::Map& map, const tmx::Object& object, TransformComponent* transform)
{
    if (!transform)
        return;

    transform->Position.x = object.getPosition().x / float(map.getTileSize().x);
    transform->Position.y = map.getTileCount().y - (object.getPosition().y / float(map.getTileSize().y));

    FindProperty("facing", object.getProperties(), transform->Facing);
}

void ReadWorldTMX(const char* fileName, World& world)
{
    tmx::SetAllowRelativePaths(true);
    tmx::SetTMXFileReadCallback(ReadXMLDataCallback, ReleaseXMLDataCallback);

    world.GetState() = WorldState::Loading;
    auto& map = world.GetMap();
    map.Clear();

    tmx::Map tmxMap;
    tmxMap.load(fileName);

    map.Size.X = tmxMap.getTileCount().x;
    map.Size.Y = tmxMap.getTileCount().y;

    size_t tileCount = tmxMap.getTileCount().x * tmxMap.getTileCount().y;
    map.Cells.resize(tileCount);

    for (auto& tileset : tmxMap.getTilesets())
    {
        map.Tilemap = TextureManager::GetTexture(tileset.getImagePath());
        map.TileSourceRects.clear();

        for (const auto& tile : tileset.getTiles())
        {
            Rectangle tileRect;
            tileRect.x = float(tile.imagePosition.x) / map.Tilemap.width;
            tileRect.y = float(tile.imagePosition.y) / map.Tilemap.height;
            tileRect.width = (float(tile.imageSize.x) + float(tile.imagePosition.x)) / map.Tilemap.width;
            tileRect.height = (float(tile.imageSize.y) + float(tile.imagePosition.y)) / map.Tilemap.height;

            map.TileSourceRects.push_back(tileRect);
        }
    }

    map.LightInfo = LightingInfo();

    auto& mapProps = tmxMap.getProperties();
    FindProperty("skybox", mapProps, map.LightInfo.SkyboxTextureName);
    FindProperty("ambient_direction_angle", mapProps, map.LightInfo.AmbientAngle);
    FindProperty("extereor_ambient_level", mapProps, map.LightInfo.ExteriorAmbientLevel);
    FindProperty("interior_ambient_level", mapProps, map.LightInfo.InteriorAmbientLevel);

    DoForEachTileInLayer(world, tmxMap, "walls", [&map](const tmx::TileLayer::Tile& tile, int x, int y, size_t mapCellIndex)
        {
            if (tile.ID != 0)
            {
                map.Cells[mapCellIndex].State = MapCellState::Wall;
                map.Cells[mapCellIndex].Tiles[0] = tile.ID - 1;
            }
        });

    DoForEachTileInLayer(world, tmxMap, "floors", [&map](const tmx::TileLayer::Tile& tile, int x, int y, size_t mapCellIndex)
        {
            if (tile.ID != 0 && map.Cells[mapCellIndex].State != MapCellState::Wall)
            {
                map.Cells[mapCellIndex].State = MapCellState::Empty;
                map.Cells[mapCellIndex].Tiles[0] = tile.ID - 1;
            }
        });

    DoForEachTileInLayer(world, tmxMap, "ceilings", [&map](const tmx::TileLayer::Tile& tile, int x, int y, size_t mapCellIndex)
        {
            if (tile.ID != 0 && map.Cells[mapCellIndex].State != MapCellState::Wall)
            {
                map.Cells[mapCellIndex].State = MapCellState::Empty;
                map.Cells[mapCellIndex].Tiles[1] = tile.ID - 1;
            }
        });

    DoForEachObjectInLayer(world, tmxMap, "objects", [&world, &map, &tmxMap](const tmx::Object& object)
        {
            auto* spawn = world.AddObject();
            SetObjectTransform(tmxMap, object, spawn->AddComponent<TransformComponent>());
            spawn->AddComponent<SpawnPointComponent>();
        },
        "spawn");

    tmx::ClearTMXFileReadCallback();
    tmx::SetAllowRelativePaths(false);

    world.GetState() = WorldState::Playing;
}