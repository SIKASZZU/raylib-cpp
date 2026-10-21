
#include "model_assets.hpp"

ModelAssets LoadModelAssets()
{
    ModelAssets models;

    // map
    models.emplace(ModelId::Wall, LoadModel("resources/wall/maze_wall.obj"));
    models.emplace(ModelId::Ground, LoadModel("resources/ground/maze_ground.obj"));
    models.emplace(ModelId::Cube, LoadModel("resources/cube.obj"));

    // asterion
    models.emplace(ModelId::AsterionLower, LoadModel("resources/asterion/griever_lower.obj"));
    models.emplace(ModelId::AsterionUpper, LoadModel("resources/asterion/griever_upper.obj"));
    models.emplace(ModelId::AsterionBody, LoadModel("resources/asterion/asterion_body_temp.obj"));

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