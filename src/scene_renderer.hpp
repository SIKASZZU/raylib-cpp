#pragma once
#include "raylib.h"
#include <vector>
#include "level.hpp" // wherever Level / WallInstance live in your project

struct RenderStats
{
    int totalInstances;
    int visibleInstances;
    int drawCalls;
};

// Spatial bucket: walls grouped by a coarse grid so whole groups can be rejected at once
struct WallCell
{
    std::vector<int> walls; // indices into level->walls
    Vector3 center;
    float radius;
};

struct SceneRenderer
{
    // ground: one mesh for the entire map (material is borrowed from your ground Model)
    Mesh groundMesh;
    Material groundMaterial;
    Matrix groundTransform;

    // per-wall data, precomputed once (walls are static)
    std::vector<Matrix> wallTransform;
    std::vector<Vector3> wallCenter;
    std::vector<float> wallRadius;

    std::vector<WallCell> cells;

    // reused every frame so there are no per-frame allocations
    std::vector<Matrix> visibleTransforms;
    std::vector<int> visibleIdx;
};

void SceneRenderer_Init(SceneRenderer &r, Model ground, const Level *level);
void SceneRenderer_Unload(SceneRenderer &r);

void DrawShadowCasters(Model wall, const Level *level);
RenderStats DrawLevel(SceneRenderer &r, Model wall, const Level *level, const Camera *camera);