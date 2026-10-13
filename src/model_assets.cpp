
#include "model_assets.hpp"

ModelAssets LoadModelAssets()
{
    ModelAssets models;
    models.emplace(ModelId::Wall, LoadModel("resources/wall/maze_wall.obj"));
    models.emplace(ModelId::Ground, LoadModel("resources/ground/maze_ground.obj"));
    models.emplace(ModelId::Cube, LoadModel("resources/cube.obj"));
    return models;
}

void UnloadModelAssets(const ModelAssets &models)
{
    for (const auto &[id, model] : models)
    {
        (void)id;
        UnloadModel(model);
    }
}