#include "map/map_reader.h"
#include "world.h"
#include "map/map.h"

#include "services/texture_manager.h"
#include "services/resource_manager.h"
#include "services/table_manager.h"
#include "services/model_manager.h"

#include "components/door_controller_component.h"
#include "components/map_object_component.h"
#include "components/mobile_object_component.h"
#include "components/mob_behavior_component.h"
#include "components/spawn_point_component.h"
#include "components/transform_component.h"
#include "components/trigger_component.h"

#include "utilities/light_utils.h"

#include "LDtkLoader/Project.hpp"

#include "tmxlite/Map.hpp"
#include "tmxlite/TileLayer.hpp"
#include "tmxlite/ObjectGroup.hpp"
#include "tmxlite/FileReader.hpp"

class MapReader
{
public:
    MapReader(World& world) :TheWorld(world), TheMap(world.GetMap()) {}

    virtual bool Read(std::string_view filename) = 0;

protected:
    World& TheWorld;
    Map& TheMap;
};

class LDTKMapReader : MapReader
{
private:
    int GridSize = 0;

    Vector2 Convert(const ldtk::IntPoint& point) const
    {
        return Vector2{ (float)point.x / GridSize, TheMap.Size.Y - ((float)point.y / GridSize) };
    }

    Vector2 ConvertNoFlip(const ldtk::IntPoint& point) const
    {
        return Vector2{ (float)point.x / GridSize, ((float)point.y / GridSize) };
    }

    size_t PositionToMapID(const ldtk::IntPoint& point)
    {
        auto coord = point;
        coord.x /= TheMap.Size.X;
        coord.y = int((TheMap.Size.Y - (coord.y / TheMap.Size.Y)) - 1);

        size_t cellIndex = size_t(coord.y * TheMap.Size.X + coord.x);

        return cellIndex;
    }

    template<class T>
    bool SetFromProperty(const std::string& name, const ldtk::FieldsContainer& container, T& value)
    {
        auto field = container.getField<T>(name);
        if (field.is_null())
            return false;

        value = field.value();

        return true;
    }

    void SetObjectTransform(const ldtk::Entity& object, TransformComponent* transform)
    {
        if (!transform)
            return;

        Vector2 convertedPos = Convert(object.getPosition());

        transform->Position.x = convertedPos.x;
        transform->Position.y = convertedPos.y;
        transform->Position.z = 0;

        SetFromProperty("Facing", object, transform->Facing);
    }

    void SetupLightInfo(const ldtk::Level & level)
    {
        TheMap.LightInfo = LightingInfo();

        SetFromProperty("skybox", level, TheMap.LightInfo.SkyboxTextureName);
        SetFromProperty("extereor_ambient_level", level, TheMap.LightInfo.ExteriorAmbientLevel);
        SetFromProperty("interior_ambient_level", level, TheMap.LightInfo.InteriorAmbientLevel);
        SetFromProperty("ambient_direction_angle", level, TheMap.LightInfo.AmbientAngle);
    }

    void ReadTileset(const ldtk::Layer& layer)
    {
        auto& tileset = layer.getTileset();

        std::string path = tileset.path;

        while (path.substr(0, 3) == "../")
            path = path.substr(3);

        TheMap.Tilemap = TextureManager::GetTexture(path);
        TheMap.TileSourceRects.clear();

        for (int i = 0; i < 255; i++)
        {
            auto point = tileset.getTileTexturePos(i);
            if (point.x >= tileset.texture_size.x || point.y >= tileset.texture_size.y)
                break;

            Rectangle tileRect;
            tileRect.x = float(point.x) / tileset.texture_size.x;
            tileRect.y = float(point.y) / tileset.texture_size.y;
            tileRect.width = (float(point.x + tileset.tile_size) / float(tileset.texture_size.x));
            tileRect.height = (float(point.y + tileset.tile_size) / float(tileset.texture_size.y));

            TheMap.TileSourceRects.push_back(tileRect);
        }
    }

    void ReadEmptyLayer(const ldtk::Layer& layer, int tileIndex)
    {
        for (auto cell : layer.allTiles())
        {
            auto coord = Convert(cell.getPosition());
            auto cellIndex = PositionToMapID(cell.getPosition());

            TheMap.Cells[cellIndex].State = MapCellState::Empty;
            TheMap.Cells[cellIndex].Tiles[tileIndex] = cell.tileId;
        }
    }

    void ReadWallsLayer(const ldtk::Layer& layer)
    {
        for (auto cell : layer.allTiles())
        {
            auto coord = Convert(cell.getPosition());
            auto cellIndex = PositionToMapID(cell.getPosition());

            TheMap.Cells[cellIndex].State = MapCellState::Wall;
            TheMap.Cells[cellIndex].Tiles[0] = cell.tileId;
        }
    }

    void AddSpawnObject(const ldtk::Entity& object)
    {
        auto* spawn = TheWorld.AddObject();

        SetObjectTransform(object, spawn->AddComponent<TransformComponent>());
        spawn->AddComponent<SpawnPointComponent>();
    }

    void AddTrigger(const ldtk::Entity& object)
    {
        Vector2 pos = Convert(object.getPosition());
        Vector2 size = ConvertNoFlip(object.getSize());

        auto* trigger = TheWorld.AddObject();
        auto* transform = trigger->AddComponent<TransformComponent>();
        transform->Position.x = pos.x;
        transform->Position.y = pos.y;

        auto* volume = trigger->AddComponent<TriggerComponent>();
        volume->Bounds = Rectangle{ pos.x, pos.y - size.y, size.x, size.y };
    }

    void AddLightZoneObject(const ldtk::Entity& object)
    {
        Vector2 pos = Convert(object.getPosition());
        Vector2 size = ConvertNoFlip(object.getSize());
        auto bounds = Rectangle{ pos.x, pos.y - size.y, size.x, size.y };

        auto sequenceTable = TableManager::GetTable(BootstrapTable)->GetFieldAsTable("light_sequences");

        LightZoneInfo zone;
        if (sequenceTable)
        {
            const auto sequence = object.getField<ldtk::EnumValue>("light_sequence");
            if (!sequence.is_null())
            {
                zone.SequenceValues = LightUtils::ParseLightSequence(sequenceTable->GetField(sequence.value().name));
            }  
        }

        SetFromProperty("max_level", object, zone.MaxLevel);
        SetFromProperty("min_level", object, zone.MinLevel);

        float len = 0;
        if (SetFromProperty("sequence_lenght", object, len) && len > 0)
        {
            zone.SequenceLenght = len;

            zone.SequenceFrameTime = zone.SequenceLenght / zone.SequenceValues.size();
        }

        for (int y = int(bounds.y); y < int(bounds.y + bounds.height); y++)
        {
            for (int x = int(bounds.x); x < int(bounds.x + bounds.width); x++)
            {
                TheMap.GetCellRef(x, y).LightZone = uint8_t(TheMap.LightZones.size());
            }
        }

        zone.Reset();

        TheMap.LightZones.push_back(zone);
    }

    void AddModelObject(const ldtk::Entity& object, ldtk::Project& project)
    {
        auto* mapObject = TheWorld.AddObject();

        SetObjectTransform(object, mapObject->AddComponent<TransformComponent>());

        auto field = object.getField<ldtk::EnumValue>("Model");

        auto* modelComp = mapObject->AddComponent<MapObjectComponent>(field.value().name);
        SetFromProperty("Solid", object, modelComp->Solid);
    }

public:
    LDTKMapReader(World& world) : MapReader(world) {}

    bool Read(std::string_view filename) override
    {
        ldtk::Project project;
        auto resource = ResourceManager::OpenResource(filename.data());
        if (!resource)
            return false;

        project.loadFromMemory(resource->DataBuffer, resource->DataSize);
        ResourceManager::ReleaseResource(resource);
        const auto& mapWorld = project.getWorld();
        const auto& level = *mapWorld.allLevels().begin();
        SetupLightInfo(level);

        const auto& floorLayer = level.getLayer("Floors");
        GridSize = floorLayer.getGridSize().x;

        ReadTileset(floorLayer);

        float mapWidth = float(level.size.x / floorLayer.getGridSize().x);
        float mapHeight = float(level.size.y / floorLayer.getGridSize().y);

        TheMap.Size.X = int(mapWidth);
        TheMap.Size.Y = int(mapHeight);
        TheMap.Cells.resize(size_t(mapWidth * mapHeight));

        ReadEmptyLayer(floorLayer, 0);
        ReadEmptyLayer(level.getLayer("Ceilings"), 1);
        ReadWallsLayer(level.getLayer("Walls"));

        const auto& objects = level.getLayer("Objects");

        for (auto& object : objects.allEntities())
        {
            if (object.getName() == "PlayerSpawn")
                AddSpawnObject(object);
            else if (object.getName() == "Model")
                AddModelObject(object, project);
            else if (object.getName() == "LightZone")
                AddLightZoneObject(object);
            else if (object.getName() == "Trigger")
                AddTrigger(object);
        }

        return true;
    }
};


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

void ReadWorld(const char* fileName, World& world)
{
    world.GetState() = WorldState::Loading;
    auto& map = world.GetMap();
    map.Clear();
    map.LightZones.clear();

    LDTKMapReader reader(world);

    reader.Read(fileName);

    world.GetState() = WorldState::Playing;
}

void ReadWorldTMX(const char* fileName, World& world)
{
  /*  world.GetMap()

   

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
            auto* mapObject = world.AddObject();
            SetObjectTransform(tmxMap, object, mapObject->AddComponent<TransformComponent>());
            auto* modelComp = mapObject->AddComponent<MobComponent>();
            mapObject->AddComponent<MobBehaviorComponent>();
        },
        "mob");

    */

    world.GetState() = WorldState::Playing;
}