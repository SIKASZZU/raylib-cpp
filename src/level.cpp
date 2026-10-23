
#include "level.hpp"
#include "map.hpp"

static bool IsWallTile(int tile)
{
    switch (tile)
    {
    case Map::WALL_CUBE:
    case Map::WALL_CUBE_GROUND:
    case Map::WALL_CUBE_SPRITE:
    case Map::INGROWN_WALL_CUBE:
    case Map::SECTOR_1_WALL_VAL:
    case Map::SECTOR_2_WALL_VAL:
    case Map::SECTOR_3_WALL_VAL:
    case Map::TUNNEL_WALL:
    case Map::TREE_TRUNK:
        return true;
    default:
        return false;
    }
}

void LevelInitialize(Level* level)
{
    srand(time(0));

    // terrainMap
    MapGenerator::init();

    level->walls.clear();
    level->groundTiles.clear();
    level->groundIndexByTile.assign(get_map_size(), -1);
    level->wallIndexByTile.assign(get_map_size(), -1);

    MapBoundsVec4 mapBounds = get_map_limits();

    for (int z = mapBounds.zMin; z < mapBounds.zMax; z++)
    {
        for (int x = mapBounds.xMin; x < mapBounds.xMax; x++)
        {
            const int tileIndex = get_map_index(x, z);
            const int tile = terrainMap[tileIndex];


            if (tile == Map::EMPTY || tile == Map::VOID_CUBE ||
                tile == Map::VOID_CUBE_NEIGHBOUR || tile == Map::INVISIBLE_CUBE)
                continue;

            const float worldX = x * MapConfig::TileSize;
            const float worldZ = z * MapConfig::TileSize;

            level->groundIndexByTile[tileIndex] = static_cast<int>(level->groundTiles.size());
            level->groundTiles.push_back({ worldX, 0.0f, worldZ });

            if (!IsWallTile(tile))
                continue;

            WallInstance wall = {};
            wall.position = { worldX, 0.0f, worldZ };
            level->wallIndexByTile[tileIndex] = static_cast<int>(level->walls.size());

            // const bool wallWest = x > mapBounds.xMin && IsWallTile(terrainMap[get_map_index(x - 1, z)]);
            // const bool wallEast = x + 1 < mapBounds.xMax && IsWallTile(terrainMap[get_map_index(x + 1, z)]);
            // const bool wallNorth = z > mapBounds.zMin && IsWallTile(terrainMap[get_map_index(x, z - 1)]);
            // const bool wallSouth = z + 1 < mapBounds.zMax && IsWallTile(terrainMap[get_map_index(x, z + 1)]);

            // always have the large face of wall towards random stuff thats not walls.
            int randomNumber = rand() % 5;
            switch (randomNumber)
            {
            case 1:
                wall.rotationDegrees = 90.0f;
                break;
            case 2:
                wall.rotationDegrees = 180.0f;
            case 3:
                wall.rotationDegrees = 270.0f;
            default:
                break;
            }

            level->walls.push_back(wall);
        }
    }
}
