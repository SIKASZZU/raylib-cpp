#ifndef SHADER_SYSTEM_H
#define SHADER_SYSTEM_H

#include "raylib.h"

typedef struct
{
    Shader lighting;
    Shader shadow;
    RenderTexture2D shadowMap;
    Camera lightCamera;
} ShaderSystem;

bool ShaderSystemInitialize(ShaderSystem *system, Model *wall, Model *ground);
void ShaderSystemUnload(ShaderSystem *system);

#endif
