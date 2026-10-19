#pragma once
#include "raylib.h"
#include "level.hpp" // wherever Level / WallInstance live
#include "model_assets.hpp"
#include "map.hpp"

#include <vector>

struct SceneItem
{
    int mapTileId;
    Vector3 position;
    float rotationDegrees;
};

struct SceneVector
{
    std::vector<SceneItem> items;
};


class Scene {
public:
    SceneVector sceneItems;
    static void draw(const ModelAssets& models, const Level* level, const Camera* camera, const SceneVector* sceneItems = nullptr);
    // can this function be erased, when stopping the rendering with f4 is not needed anymore?
    // can it be only called from draw and that's it?
    static SceneVector createSceneVector(const Level* level, const Camera* camera);
};
