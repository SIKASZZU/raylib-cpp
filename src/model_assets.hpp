// model_assets.hpp
#pragma once
#include "raylib.h"
#include <unordered_map>

enum class ModelId
{
    Wall,
    Ground,
    Cube,
};

using ModelAssets = std::unordered_map<ModelId, Model>;

ModelAssets LoadModelAssets();
float GetScaleToFitGround(const Model& model);
void UnloadModelAssets(const ModelAssets &models);