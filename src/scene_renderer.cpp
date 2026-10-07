#include "scene_renderer.hpp"
#include "game_config.hpp" // NEAR_PLANE / LIGHT_FAR live here, don't redefine them
#include "raymath.h"

#include <math.h>
#include <algorithm>

// Camera basis and tangents, computed once per frame instead of once per object.
struct ViewFrustum
{
    Vector3 pos, fwd, right, up;
    float tanH, tanV, secH, secV;
};

static ViewFrustum MakeFrustum(const Camera &cam)
{
    ViewFrustum f;
    f.pos = cam.position;
    f.fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    f.right = Vector3Normalize(Vector3CrossProduct(f.fwd, cam.up));
    f.up = Vector3Normalize(Vector3CrossProduct(f.right, f.fwd));
    f.tanV = tanf(cam.fovy * DEG2RAD * 0.5f);
    f.tanH = f.tanV * ((float)GetScreenWidth() / (float)GetScreenHeight());
    f.secV = sqrtf(1.0f + f.tanV * f.tanV);
    f.secH = sqrtf(1.0f + f.tanH * f.tanH);
    return f;
}

static bool SphereVisible(const ViewFrustum &f, Vector3 center, float radius)
{
    const Vector3 o = Vector3Subtract(center, f.pos);
    const float depth = Vector3DotProduct(o, f.fwd);

    if (depth + radius < LIGHT_NEAR)
        return false;
    if (depth - radius > LIGHT_FAR)
        return false;

    const float h = fabsf(Vector3DotProduct(o, f.right));
    const float v = fabsf(Vector3DotProduct(o, f.up));
    return h <= depth * f.tanH + radius * f.secH &&
           v <= depth * f.tanV + radius * f.secV;
}

RenderStats DrawLevel(Model wall, Model ground, const Level *level, const Camera *camera)
{
    RenderStats stats = {};
    const ViewFrustum f = MakeFrustum(*camera);

    // ---- ground: tile based, but only tiles within the far plane are even considered ----
    const float T = LEVEL_TILE_SIZE;
    const float groundScale = GROUND_MODEL_SCALE;
    const float groundRadius = T * 0.7072f; // half of the tile diagonal

    const int xMin = std::max(-MAP_SIDE_LENGTH, (int)floorf((camera->position.x - LIGHT_FAR) / T));
    const int xMax = std::min(MAP_SIDE_LENGTH - 1, (int)ceilf((camera->position.x + LIGHT_FAR) / T));
    const int zMin = std::max(-MAP_SIDE_LENGTH, (int)floorf((camera->position.z - LIGHT_FAR) / T));
    const int zMax = std::min(MAP_SIDE_LENGTH - 1, (int)ceilf((camera->position.z + LIGHT_FAR) / T));

    stats.totalInstances += (2 * MAP_SIDE_LENGTH) * (2 * MAP_SIDE_LENGTH);

    for (int z = zMin; z <= zMax; z++)
    {
        for (int x = xMin; x <= xMax; x++)
        {
            const Vector3 position = {x * T, 0.0f, z * T};
            if (!SphereVisible(f, Vector3{position.x, -0.5f * groundScale, position.z}, groundRadius))
                continue;

            DrawModel(ground, position, groundScale, WHITE);
            stats.visibleInstances++;
            stats.drawCalls++;
        }
    }

    // ---- walls: plain DrawModelEx for everything inside the frustum ----
    for (const WallInstance &w : level->walls)
    {
        stats.totalInstances++;
        const float halfLength = WALL_HALF_LENGTH;
        const float halfThickness = WALL_HALF_THICKNESS;
        const float height = WALL_HEIGHT;
        const float radius = sqrtf(halfLength * halfLength + halfThickness * halfThickness + height * height * 0.25f);
        const Vector3 center = {w.position.x, w.position.y + height * 0.5f, w.position.z};

        if (!SphereVisible(f, center, radius))
            continue;

        DrawModelEx(wall, w.position, Vector3{0.0f, 1.0f, 0.0f}, w.rotationDegrees,
                    Vector3{WALL_MODEL_SCALE, WALL_MODEL_SCALE, WALL_MODEL_SCALE}, WHITE);
        stats.visibleInstances++;
        stats.drawCalls++;
    }

    // ---- sun: relative to the camera so the far plane never clips it ----
    const Vector3 sunOffset = {300.0f, 300.0f, 0.0f};
    const float sunDist = LIGHT_FAR * 0.9f;
    const float sunRadius = 100.0f * sunDist / Vector3Length(sunOffset);
    DrawSphere(Vector3Add(camera->position, Vector3Scale(Vector3Normalize(sunOffset), sunDist)),
               sunRadius, Color{255, 215, 0, 255});
    stats.drawCalls++;

    return stats;
}