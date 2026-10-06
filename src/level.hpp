#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"

#include <vector>

struct WallInstance
{
    Vector3 position;
    float rotationDegrees;
    float scale;
};

struct Level
{
    std::vector<WallInstance> walls;
};

void LevelInitialize(Level *level);

#endif
