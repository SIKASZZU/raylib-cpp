
#include "model_assets.hpp"

ModelAssets LoadModelAssets()
{
    ModelAssets models;

    // map
    models.emplace(ModelId::Wall, LoadModel("resources/wall/maze_wall.obj"));
    models.emplace(ModelId::Ground, LoadModel("resources/ground/maze_ground.obj"));
    models.emplace(ModelId::Cube, LoadModel("resources/cube.obj"));

    // robot
    models.emplace(ModelId::robotLower, LoadModel("resources/robot/lower.obj"));
    models.emplace(ModelId::robotUpper, LoadModel("resources/robot/upper.obj"));
    models.emplace(ModelId::robotBody, LoadModel("resources/robot/body_temp.obj"));

    return models;
}

void UnloadModelAssets(const ModelAssets& models)
{
    for (const auto& [id, model] : models)
    {
        (void)id;
        UnloadModel(model);
    }
}