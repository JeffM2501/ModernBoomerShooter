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

    Rectangle ObjectBoundsToRect(const ldtk::Entity& object)
    {
        Vector2 pos = Convert(object.getPosition());
        Vector2 size = ConvertNoFlip(object.getSize());
        return Rectangle{ pos.x, pos.y - size.y, size.x, size.y };
    }


    Rectangle ObjectBoundsToRect(const ldtk::EntityRef& objectRef)
    {
        Vector2 pos = Convert(objectRef->getPosition());
        Vector2 size = ConvertNoFlip(objectRef->getSize());
        return Rectangle{ pos.x, pos.y - size.y, size.x, size.y };
    }

    uint8_t GetTileFromRect(int x, int y, int w, int h)
    {
        float scaleX = x / (float)TheMap.Tilemap.width;
        float scaleY = y / (float)TheMap.Tilemap.height;

        float epsilonX = 1 / (float)TheMap.Tilemap.width;
        float epsilonY = 1 / (float)TheMap.Tilemap.height;

        for (uint8_t index = 0; index < TheMap.TileSourceRects.size(); index++)
        {
            Rectangle& rect = TheMap.TileSourceRects[index];
            if (fabsf(rect.x - scaleX) < epsilonX && fabs(rect.y - scaleY) < epsilonY)
                return index;
        }

        return 0;
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

    template<class T>
    bool SetFromProperty(const std::string& name, const ldtk::EntityRef& containerRef, T& value)
    {
        auto field = containerRef->getField<T>(name);
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

        float angle = 0;
        if (SetFromProperty("Facing", object, angle))
            transform->SetFacing(angle);
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

    void AddMobObject(const ldtk::Entity& object)
    {
        auto* mobObject = TheWorld.AddObject();
        SetObjectTransform(object, mobObject->AddComponent<TransformComponent>());
        auto* modelComp = mobObject->AddComponent<MobComponent>();
        auto* behavior = mobObject->AddComponent<MobBehaviorComponent>();

        SetFromProperty("FollowPath", object, behavior->FollowPath);
        SetFromProperty("MoveSpeed", object, behavior->MoveSpeed);
        SetFromProperty("RotationSpeed", object, behavior->RotationSpeed);

        for (auto& point : object.getArrayField<ldtk::IntPoint>("Path"))
        {
            Vector3 pointPos = {point.value().x + 0.5f, (TheMap.Size.Y - (point.value().y)) - 0.5f, 0 };

            behavior->Path.push_back(pointPos);
        }
    }

    void AddTrigger(const ldtk::Entity& object)
    {
        auto* trigger = TheWorld.AddObject();
        auto* volume = trigger->AddComponent<TriggerComponent>();
        volume->Bounds = ObjectBoundsToRect(object);
        SetFromProperty("trigger_id", object, volume->TriggerId);
    }

    void AddLightZoneObject(const ldtk::Entity& object)
    {
        auto bounds = ObjectBoundsToRect(object);

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

    void AddDoorObject(const ldtk::Entity& object)
    {
        auto triggerBounds = ObjectBoundsToRect(object);

        auto trigger = TheWorld.AddObject();

        trigger->AddComponent<TriggerComponent>(triggerBounds);
        DoorControllerComponent* doorController = trigger->AddComponent<DoorControllerComponent>();

        SetFromProperty("FullyOpenBeforeClose", object, doorController->MustOpenBeforClose);
        SetFromProperty("OpenSpeed", object, doorController->OpenSpeed);
        SetFromProperty("CloseSpeed", object, doorController->CloseSpeed);
        SetFromProperty("StayOpen", object, doorController->StayOpen);
        SetFromProperty("MinimumOpenTime", object, doorController->MiniumOpenTime);

        // get doors
        for (auto doorRef : object.getArrayField<ldtk::EntityRef>("Doors"))
        {
            if (doorRef.is_null())
                continue;

            const auto& door = doorRef.value();

            auto doorBounds = ObjectBoundsToRect(door);

            Vector2 center = { doorBounds.x, doorBounds.y };
            
            int doorX = int(center.x);
            int doorY = int(center.y);


            auto grid = door->getGridPosition();
            grid.y = (TheMap.Size.Y - grid.y-1);


            auto& doorCell = TheMap.GetCellRef(grid.x, grid.y);
            doorCell.State = MapCellState::Door;

            const auto& doorTexture = door->getField<ldtk::TileRef>("Texture");

            if (!doorTexture.is_null())
                doorCell.Tiles[2] = GetTileFromRect(doorTexture.value().bounds.x, doorTexture.value().bounds.y, doorTexture.value().bounds.width, doorTexture.value().bounds.height);
            
            if (door->getName() == "Door_X")
                doorCell.Flags |= MapCellFlags::XAllignment;

            bool backwards = false;
            bool vertical = false;

            SetFromProperty("Backwards", door, backwards);
            SetFromProperty("Vertical", door, vertical);

            if (backwards)
                doorCell.Flags |= MapCellFlags::Reversed;

            if (vertical)
                doorCell.Flags |= MapCellFlags::HorizontalVertical;

            doorCell.Flags |= MapCellFlags::Impassible;

            doorController->Doors.push_back(TheMap.GetCellIndex(grid.x, grid.y));
        }

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
            else if (object.getName() == "MOB")
                AddMobObject(object);
            else if (object.getName() == "DoorTrigger")
                AddDoorObject(object);
        }

        return true;
    }
};

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

