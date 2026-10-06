#include "level.hpp"

#include "game_config.hpp"

void LevelInitialize(Level *level)
{
    level->walls.clear();

    for (int z = -MAP_SIDE_LENGTH; z < MAP_SIDE_LENGTH; z++)
    {
        for (int x = -MAP_SIDE_LENGTH; x < MAP_SIDE_LENGTH; x++)
        {
            WallInstance wall = {};
            wall.position = {x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE};

            if ((z & 1) && (x & 1))
            {
                wall.position.y = 5.0f;
                wall.rotationDegrees = ROTATED_WALL_ANGLE;
                wall.scale = 1.0f;
                level->walls.push_back(wall);
            }
            else if (!(z & 1) && !(x & 1))
            {
                wall.scale = NORMAL_WALL_SCALE;
                level->walls.push_back(wall);
            }
        }
    }
}
