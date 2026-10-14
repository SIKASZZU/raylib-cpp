#pragma once
#include "raylib.h"
#include "level.hpp" // wherever Level / WallInstance live
#include "model_assets.hpp"

struct RenderStats
{
    int totalInstances; // how you know that this is truth, ps you dont
    int visibleInstances;
    int drawCalls;
};

RenderStats DrawLevel(const ModelAssets &models, const Level *level, const Camera *camera);