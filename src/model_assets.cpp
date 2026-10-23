
#include "model_assets.hpp"

ModelAssets LoadModelAssets()
{
    ModelAssets models;
    // models.emplace(ModelId::Wall, LoadModel("resources/wall/maze_wall.obj"));
    models.emplace(ModelId::Wall, LoadModel("resources/wall/maze_wall_collision_temp.obj"));

    models.emplace(ModelId::Ground, LoadModel("resources/ground/maze_ground.obj"));
    models.emplace(ModelId::Cube, LoadModel("resources/cube.obj"));
    return models;
}

float GetScaleToFitGround(const Model& model)
{
    const BoundingBox bounds = GetModelBoundingBox(model);
    const float width = bounds.max.x - bounds.min.x;
    const float depth = bounds.max.z - bounds.min.z;
    const float height = std::max(bounds.max.y, bounds.min.y);
    const float modelSize = std::max(width, depth);

    return modelSize;
}

void UnloadModelAssets(const ModelAssets& models)
{
    for (const auto& [id, model] : models)
    {
        (void)id;
        UnloadModel(model);
    }
}