#ifndef SCENE_RENDERER_H
#define SCENE_RENDERER_H

#include "raylib.h"

typedef struct
{
    int totalInstances;
    int visibleInstances;
} RenderStats;

RenderStats DrawLevel(Model wall, Model ground, const Camera *camera);
void DrawShadowCasters(Model wall, Model ground);

#endif
