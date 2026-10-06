#include "scene_renderer.hpp"

#include "game_config.hpp"
#include "raymath.h"

#include <math.h>

static bool IsSphereInCameraView(const Camera *camera, Vector3 center, float radius);

void DrawShadowCasters(Model wall, Model ground)
{
    const float groundScale = LEVEL_TILE_SIZE / GROUND_MODEL_SIZE;
    for (int z = -MAP_SIDE_LENGTH; z < MAP_SIDE_LENGTH; z++)
    {
        for (int x = -MAP_SIDE_LENGTH; x < MAP_SIDE_LENGTH; x++)
        {
            Vector3 position = {x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE};
            DrawModel(ground, position, groundScale, WHITE);

            if ((z & 1) && (x & 1))
            {
                DrawModelEx(wall, position, Vector3{0.0f, 1.0f, 0.0f},
                            ROTATED_WALL_ANGLE, Vector3{1.0f, 1.0f, 1.0f}, WHITE);
            }
            else if (!(z & 1) && !(x & 1))
            {
                DrawModel(wall, position, NORMAL_WALL_SCALE, WHITE);
            }
        }
    }
}

RenderStats DrawLevel(Model wall, Model ground, const Camera *camera)
{
    const float groundScale = LEVEL_TILE_SIZE / GROUND_MODEL_SIZE;
    const float groundRadius = sqrtf(2.0f * LEVEL_TILE_SIZE * LEVEL_TILE_SIZE * 0.25f +
                                     0.25f * groundScale * groundScale);
    const float wallRadiusBase = sqrtf(WALL_HALF_LENGTH * WALL_HALF_LENGTH +
                                       (WALL_HEIGHT * 0.5f + 0.2f) * (WALL_HEIGHT * 0.5f + 0.2f) +
                                       1.4f * 1.4f);
    RenderStats stats = {};

    for (int z = -MAP_SIDE_LENGTH; z < MAP_SIDE_LENGTH; z++)
    {
        for (int x = -MAP_SIDE_LENGTH; x < MAP_SIDE_LENGTH; x++)
        {
            Vector3 position = {x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE};
            stats.totalInstances++;

            // ground
            if (IsSphereInCameraView(camera, Vector3{position.x, -0.5f * groundScale, position.z}, groundRadius))
            {
                DrawModel(ground, position, groundScale, WHITE);
                stats.visibleInstances++;
            }

            // rotated walls
            if ((z & 1) && (x & 1))
            {
                stats.totalInstances++;
                if (IsSphereInCameraView(camera, Vector3{position.x, WALL_HEIGHT * 0.5f, position.z}, wallRadiusBase))
                {
                    position.y += 5.0f;
                    DrawModelEx(wall, position, Vector3{0.0f, 1.0f, 0.0f},
                                ROTATED_WALL_ANGLE, Vector3{1.0f, 1.0f, 1.0f}, WHITE);
                    stats.visibleInstances++;
                }
            }

            // normal walls
            else if (!(z & 1) && !(x & 1))
            {
                const float wallScale = NORMAL_WALL_SCALE;
                stats.totalInstances++;
                if (IsSphereInCameraView(camera,
                                         Vector3{position.x, WALL_HEIGHT * wallScale * 0.5f, position.z},
                                         wallRadiusBase * wallScale))
                {
                    DrawModel(wall, position, wallScale, WHITE);
                    stats.visibleInstances++;
                }
            }
        }
    }

    const Vector3 sunPosition = {300.0f, 300.0f, 0.0f};
    stats.totalInstances++;
    if (IsSphereInCameraView(camera, sunPosition, 100.0f))
    {
        DrawSphere(sunPosition, 100.0f, (Color){255, 215, 0, 255});
        stats.visibleInstances++;
    }

    return stats;
}

static bool IsSphereInCameraView(const Camera *camera, Vector3 center, float radius)
{
    const Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera->up));
    const Vector3 cameraUp = Vector3Normalize(Vector3CrossProduct(right, forward));
    const Vector3 offset = Vector3Subtract(center, camera->position);
    const float depth = Vector3DotProduct(offset, forward);

    if (depth + radius < 0.01f)
        return false;

    const float tanVertical = tanf(camera->fovy * DEG2RAD * 0.5f);
    const float tanHorizontal = tanVertical * ((float)GetScreenWidth() / GetScreenHeight());
    const float horizontal = fabsf(Vector3DotProduct(offset, right));
    const float vertical = fabsf(Vector3DotProduct(offset, cameraUp));

    if (horizontal > depth * tanHorizontal + radius * sqrtf(1.0f + tanHorizontal * tanHorizontal))
        return false;
    return vertical <= depth * tanVertical + radius * sqrtf(1.0f + tanVertical * tanVertical);
}
