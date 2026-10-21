// model_assets.hpp
#pragma once
#include "raylib.h"
#include <unordered_map>

enum class ModelId
{
    Wall,
    Ground,
    Cube,
    robotBody,
    robotUpper,
    robotLower,
};

using ModelAssets = std::unordered_map<ModelId, Model>;

ModelAssets LoadModelAssets();
void UnloadModelAssets(const ModelAssets& models);