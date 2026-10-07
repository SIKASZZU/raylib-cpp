#pragma once
#include "raylib.h"
#include "level.hpp" // wherever Level / WallInstance live

struct RenderStats
{
    int totalInstances;
    int visibleInstances;
    int drawCalls;
};

RenderStats DrawLevel(Model wall, Model ground, const Level *level, const Camera *camera);