#include "map/map_reader.h"
#include "world.h"
#include "map/map.h"

#include "services/texture_manager.h"
#include "services/resource_manager.h"
#include "services/table_manager.h"
#include "services/model_manager.h"

#include "components/spawn_point_component.h"
#include "components/transform_component.h"
#include "components/map_object_component.h"
#include "components/trigger_component.h"
#include "components/door_controller_component.h"

#include "utilities/light_utils.h"

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

using LayerTileCallback = std::function<void(const tmx::TileLayer::Tile&, MapCoordinate, MapCoordinate, size_t)>;


static size_t ComputeTMXIndex(int x, int y, MapCoordinate mapSize)
{
    return (mapSize.Y - y - 1)* mapSize.X + x;
}

void DoForEachTile(World& world, tmx::TileLayer& layer, LayerTileCallback callback)
{
    if (!callback)
        return;

    auto& tiles = layer.getTiles();

    auto mapSize = world.GetMap().Size;

    MapCoordinate tmxCoordinate(0, 0);
    MapCoordinate mapCoordinate(0, mapSize.Y-1);

    for (size_t i = 0; i < tiles.size(); i++)
    {
        size_t mapCellIndex = ComputeTMXIndex(tmxCoordinate.X, tmxCoordinate.Y, mapSize);

        callback(tiles[i], tmxCoordinate, mapCoordinate, mapCellIndex);

        tmxCoordinate.X++;
        mapCoordinate.X++;

        if (tmxCoordinate.X >= mapSize.X)
        {
            mapCoordinate.Y--;
            tmxCoordinate.Y++;

            mapCoordinate.X = tmxCoordinate.X = 0;
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
        if (property.getType() != tmx::Property::Type::String)
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
        if (property.getType() != tmx::Property::Type::Float)
            continue;

        if (property.getName() == name)
        {
            floatValue = property.getFloatValue();
            return true;
        }
    }

    return false;
}


bool FindProperty(std::string_view name, const std::vector<tmx::Property>& properties, bool& boolValue)
{
    for (auto& property : properties)
    {
        if (property.getType() != tmx::Property::Type::Boolean)
            continue;

        if (property.getName() == name)
        {
            boolValue = property.getBoolValue();
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

    FindProperty("height", object.getProperties(), transform->Position.z);

    FindProperty("facing", object.getProperties(), transform->Facing);
}

Rectangle ConvertObjectAABBToRect(const tmx::Map& map, const tmx::FloatRect& rect)
{
    Rectangle outRect;
    outRect.x = rect.left / float(map.getTileSize().x);
    outRect.y = map.getTileCount().y - (rect.top / float(map.getTileSize().y));

    outRect.width = rect.width / float(map.getTileSize().x);
    outRect.height = rect.height / float(map.getTileSize().y);

    outRect.y -= outRect.height;

    return outRect;
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

    DoForEachTileInLayer(world, tmxMap, "floors", [&map](const tmx::TileLayer::Tile& tile, MapCoordinate tmxCoord, MapCoordinate mapCoord, size_t mapCellIndex)
        {
            if (tile.ID != 0 && map.Cells[mapCellIndex].State != MapCellState::Wall)
            {
                map.Cells[mapCellIndex].State = MapCellState::Empty;
                map.Cells[mapCellIndex].Tiles[0] = tile.ID - 1;
            }
        });

    DoForEachTileInLayer(world, tmxMap, "ceilings", [&map](const tmx::TileLayer::Tile& tile, MapCoordinate tmxCoord, MapCoordinate mapCoord, size_t mapCellIndex)
        {
            if (tile.ID != 0 && map.Cells[mapCellIndex].State != MapCellState::Wall)
            {
                map.Cells[mapCellIndex].State = MapCellState::Empty;
                map.Cells[mapCellIndex].Tiles[1] = tile.ID - 1;
            }
        });


    DoForEachTileInLayer(world, tmxMap, "walls", [&map](const tmx::TileLayer::Tile& tile, MapCoordinate tmxCoord, MapCoordinate mapCoordy, size_t mapCellIndex)
        {
            if (tile.ID != 0)
            {
                map.Cells[mapCellIndex].State = MapCellState::Wall;
                map.Cells[mapCellIndex].Tiles[0] = tile.ID - 1;
            }
        });

    DoForEachTileInLayer(world, tmxMap, "doors", [&world, &map](const tmx::TileLayer::Tile& tile, MapCoordinate tmxCoord, MapCoordinate mapCoord, size_t mapCellIndex)
        {
            if (tile.ID != 0 && map.Cells[mapCellIndex].State == MapCellState::Empty)
            {
                map.Cells[mapCellIndex].State = MapCellState::Door;
                map.Cells[mapCellIndex].Tiles[2] = tile.ID - 1;

                bool isXAlligned = false;

                if (map.Cells[ComputeTMXIndex(tmxCoord.X + 1, tmxCoord.Y, map.Size)].State != MapCellState::Empty)
                    isXAlligned = true;

                if (isXAlligned)
                    map.Cells[mapCellIndex].Flags |= MapCellFlags::XAllignment;
               
                auto trigger = world.AddObject();
                Rectangle triggerBounds = { 0,0 };
                if (isXAlligned)
                {
                    triggerBounds.y = float(mapCoord.Y - 1);
                    triggerBounds.x = float(mapCoord.X);
                    triggerBounds.height = 3;
                    triggerBounds.width = 1;
                }
                else
                {
                    triggerBounds.y = float(mapCoord.Y);
                    triggerBounds.x = float(mapCoord.X-1);
                    triggerBounds.height = 1;
                    triggerBounds.width = 3;
                }

                trigger->AddComponent<TriggerComponent>(triggerBounds);
                trigger->AddComponent<DoorControllerComponent>(mapCellIndex);
                map.Cells[mapCellIndex].Flags |= MapCellFlags::Impassible;

                map.DoorCells.push_back(mapCellIndex);
            }
        });

    DoForEachObjectInLayer(world, tmxMap, "objects", [&world, &map, &tmxMap](const tmx::Object& object)
        {
            auto* spawn = world.AddObject();
            SetObjectTransform(tmxMap, object, spawn->AddComponent<TransformComponent>());
            spawn->AddComponent<SpawnPointComponent>();
        },
        "spawn");

    DoForEachObjectInLayer(world, tmxMap, "objects", [&world, &map, &tmxMap](const tmx::Object& object)
        {
            auto* mapObject = world.AddObject();
            SetObjectTransform(tmxMap, object, mapObject->AddComponent<TransformComponent>());

            std::string model;
            FindProperty("model", object.getProperties(), model);
            auto* modelComp = mapObject->AddComponent<MapObjectComponent>(model);
            FindProperty("solid", object.getProperties(), modelComp->Solid);
        },
        "object");

    DoForEachObjectInLayer(world, tmxMap, "objects", [&world, &map, &tmxMap](const tmx::Object& object)
        {
            auto* trigger = world.AddObject();
            SetObjectTransform(tmxMap, object, trigger->AddComponent<TransformComponent>());
            auto* volume = trigger->AddComponent<TriggerComponent>();
            volume->Bounds = ConvertObjectAABBToRect(tmxMap, object.getAABB());
        },
        "trigger");

    auto sequenceTable = TableManager::GetTable(BootstrapTable)->GetFieldAsTable("light_sequences");

    map.LightZones.clear();

    DoForEachObjectInLayer(world, tmxMap, "lightzone", [&world, &map, &tmxMap, sequenceTable](const tmx::Object& object)
        {
            LightZoneInfo zone;
            if (sequenceTable)
            {
                std::string sequence;
                if (FindProperty("light_sequence", object.getProperties(), sequence))
                    zone.SequenceValues = LightUtils::ParseLightSequence(sequenceTable->GetField(sequence));
            }

            FindProperty("max_level", object.getProperties(), zone.MaxLevel);
            FindProperty("min_level", object.getProperties(), zone.MinLevel);
            
            if (FindProperty("sequence_lenght", object.getProperties(), zone.SequenceLenght))
            {
                zone.SequenceFrameTime = zone.SequenceLenght / zone.SequenceValues.size();
            }
            
            auto bounds = ConvertObjectAABBToRect(tmxMap, object.getAABB());

            for (int y = int(bounds.y); y < int(bounds.y + bounds.height); y++)
            {
                for (int x = int(bounds.x); x < int(bounds.x + bounds.width); x++)
                {
                    map.GetCellRef(x, y).LightZone = uint8_t(map.LightZones.size());
                }
            }

            zone.Reset();

            map.LightZones.push_back(zone);
        },
        "zone");

    tmx::ClearTMXFileReadCallback();
    tmx::SetAllowRelativePaths(false);

    world.GetState() = WorldState::Playing;
}