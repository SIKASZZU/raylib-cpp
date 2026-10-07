#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"

#include <vector>

struct WallInstance
{
    Vector3 position;
    float rotationDegrees;
};

struct Level
{
    std::vector<WallInstance> walls;
    std::vector<Vector3> groundTiles;
    std::vector<int> groundIndexByTile;
    std::vector<int> wallIndexByTile;
    int mapWidth = 0;
};

void LevelInitialize(Level *level);

#endif
