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

    map.Cells.resize(tmxMap.getTileCount().x * tmxMap.getTileCount().y);

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

    map.SkyboxTextureName.clear();

    for (auto& property : tmxMap.getProperties())
    {
        if (property.getName() == "skybox")
            map.SkyboxTextureName = property.getStringValue();
    }

    for (auto& layer : tmxMap.getLayers())
    {
        if (layer->getClass() == "walls")
        {
            tmx::TileLayer& wallLayer = layer->getLayerAs<tmx::TileLayer>();

            auto& chunk = wallLayer.getTiles();

            for (size_t i = 0; i < wallLayer.getTiles().size(); i++)
            {
                if (chunk[i].ID != 0)
                {
                    map.Cells[i].State = MapCellState::Wall;
                    map.Cells[i].Tiles[0] = chunk[i].ID-1;
                }
            }
        }
        else if (layer->getClass() == "floors")
        {
            tmx::TileLayer& floorLayer = layer->getLayerAs<tmx::TileLayer>();

            auto& chunk = floorLayer.getTiles();

            for (size_t i = 0; i < chunk.size(); i++)
            {
                if (chunk[i].ID != 0 && map.Cells[i].State != MapCellState::Wall)
                {
                    map.Cells[i].State = MapCellState::Empty;
                    map.Cells[i].Tiles[0] = chunk[i].ID - 1;
                }
            }
        }
        else if (layer->getClass() == "ceilings")
        {
            tmx::TileLayer& ceilingLayer = layer->getLayerAs<tmx::TileLayer>();

            auto& chunk = ceilingLayer.getTiles();

            for (size_t i = 0; i < chunk.size(); i++)
            {
                if (chunk[i].ID != 0 && map.Cells[i].State != MapCellState::Wall)
                {
                    map.Cells[i].State = MapCellState::Empty;
                    map.Cells[i].Tiles[1] = chunk[i].ID - 1;
                }
            }
        }
        else if (layer->getClass() == "objects")
        {
            tmx::ObjectGroup& objectLayer = layer->getLayerAs<tmx::ObjectGroup>();

            for (auto& object : objectLayer.getObjects())
            {
                if (object.getClass() == "spawn")
                {
                    auto* spawn = world.AddObject();
                    auto* transform = spawn->AddComponent<TransformComponent>();
                    transform->Position.x = object.getPosition().x / float(tmxMap.getTileSize().x);
                    transform->Position.y = object.getPosition().y / float(tmxMap.getTileSize().y);

                    for (auto& prop : object.getProperties())
                    {
                        if (prop.getName() == "facing")
                        {
                            transform->Facing = prop.getFloatValue();
                        }
                    }

                    spawn->AddComponent<SpawnPointComponent>();
                }
            }

        }
    }

    tmx::ClearTMXFileReadCallback();
    tmx::SetAllowRelativePaths(false);

    world.GetState() = WorldState::Playing;
}