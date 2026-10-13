#pragma once
#include "raylib.h"
#include "level.hpp" // wherever Level / WallInstance live
#include "model_assets.hpp"

struct RenderStats
{
    int totalInstances;
    int visibleInstances;
    int drawCalls;
};

RenderStats DrawLevel(const ModelAssets &models, const Level *level, const Camera *camera);