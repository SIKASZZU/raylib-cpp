#pragma once
#include "raylib.h"
#include "level.hpp" // wherever Level / WallInstance live
#include "model_assets.hpp"

#include <vector>

enum class RenderItemType
{
    Ground,
    Door,
    Wall,
};

struct RenderItem
{
    RenderItemType type;
    Vector3 position;
    float rotationDegrees;
};

struct RenderSnapshot
{
    std::vector<RenderItem> items;
};

struct RenderStats
{
    int totalInstances; // how you know that this is truth, ps you dont
    int visibleInstances;
    int drawCalls;
};

RenderSnapshot CaptureRenderSnapshot(const Level *level, const Camera *camera);
RenderStats DrawLevel(const ModelAssets &models, const Level *level, const Camera *camera,
                      const RenderSnapshot *renderSnapshot = nullptr);