#include "scene_renderer.hpp"
#include "rlgl.h"
#include "game_config.hpp"
#include "raymath.h"

#include <math.h>

static bool IsSphereInCameraView(const Camera *camera, Vector3 center, float radius);

void DrawShadowCasters(Model wall, Model ground, const Level *level)
{
    const float groundScale = LEVEL_TILE_SIZE / GROUND_MODEL_SIZE;
    for (int z = -MAP_SIDE_LENGTH; z < MAP_SIDE_LENGTH; z++)
    {
        for (int x = -MAP_SIDE_LENGTH; x < MAP_SIDE_LENGTH; x++)
        {
            Vector3 position = {x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE};
            DrawModel(ground, position, groundScale, WHITE);
        }
    }

    for (const WallInstance &instance : level->walls)
    {
        DrawModelEx(wall, instance.position, Vector3{0.0f, 1.0f, 0.0f},
                    instance.rotationDegrees,
                    Vector3{instance.scale, instance.scale, instance.scale}, WHITE);
    }
}

RenderStats DrawLevel(Model wall, Model ground, const Level *level, const Camera *camera)
{
    const float groundScale = LEVEL_TILE_SIZE / GROUND_MODEL_SIZE;
    const float groundRadius = sqrtf(2.0f * LEVEL_TILE_SIZE * LEVEL_TILE_SIZE * 0.25f +
                                     0.25f * groundScale * groundScale);
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
        }
    }

    for (const WallInstance &instance : level->walls)
    {
        stats.totalInstances++;
        const float halfLength = WALL_HALF_LENGTH * instance.scale;
        const float halfThickness = WALL_HALF_THICKNESS * instance.scale;
        const float height = WALL_HEIGHT * instance.scale;
        const float wallRadius = sqrtf(halfLength * halfLength +
                                       halfThickness * halfThickness +
                                       height * height * 0.25f);
        const Vector3 center = {instance.position.x,
                                instance.position.y + height * 0.5f,
                                instance.position.z};

        if (IsSphereInCameraView(camera, center, wallRadius))
        {
            DrawModelEx(wall, instance.position, Vector3{0.0f, 1.0f, 0.0f},
                        instance.rotationDegrees,
                        Vector3{instance.scale, instance.scale, instance.scale}, WHITE);
            stats.visibleInstances++;
            rlPushMatrix();
            rlTranslatef(instance.position.x,
                         instance.position.y + height * 0.5f,
                         instance.position.z);
            rlRotatef(instance.rotationDegrees, 0.0f, 1.0f, 0.0f);
            DrawCubeWires({0.0f, 0.0f, 0.0f},
                          WALL_HALF_LENGTH * instance.scale * 2.0f,
                          height,
                          WALL_HALF_THICKNESS * instance.scale * 2.0f,
                          RED);
            rlPopMatrix();
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
