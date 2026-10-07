#ifndef SHADER_SYSTEM_H
#define SHADER_SYSTEM_H

#include "raylib.h"
#include "level.hpp"

typedef struct
{
    Shader lighting;
    Shader depth;
    RenderTexture2D shadowMap;
    Camera lightCamera;
    int shadowMapLocation;
    int showShadowMaskLocation;
} ShaderSystem;

bool ShaderSystemInitialize(ShaderSystem *system, Model *wall, Model *ground, const Level *level);
void ShaderSystemUpdateShadowMap(ShaderSystem *system, Model *wall, const Level *level);
void ShaderSystemBeginLighting(ShaderSystem *system);
void ShaderSystemEndLighting(void);
void ShaderSystemSetShadowMask(ShaderSystem *system, bool enabled);
void ShaderSystemUnload(ShaderSystem *system);

#endif
