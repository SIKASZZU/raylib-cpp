#include <math.h>
#include <algorithm>

#include "scene_renderer.hpp"
#include "game_config.hpp"
#include "map.hpp"

#include <raymath.h>

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

RenderStats DrawLevel(const ModelAssets &models, const Level *level, const Camera *camera)
{
    RenderStats stats = {};
    const ViewFrustum f = MakeFrustum(*camera);
    const Model &ground = models.at(ModelId::Ground);
    const Model &wall = models.at(ModelId::Wall);
    const Model &cube = models.at(ModelId::Cube);

    // ---- ground: tile based, but only tiles within the far plane are even considered ----
    const float groundScale = GROUND_MODEL_SCALE;

    const float T = LEVEL_TILE_SIZE;
    const float groundRadius = LEVEL_TILE_SIZE * 0.7071f; // kui visual artifact renderimises, siis asenda 0.7072-ga. // half of the tile diagonal

    MapBoundsVec4 mapBounds = get_map_limits_with_camera(camera);

    stats.totalInstances += (2 * MAP_SIDE_LENGTH) * (2 * MAP_SIDE_LENGTH);

    for (int z = mapBounds.zMin; z < mapBounds.zMax; z++)
    {
        for (int x = mapBounds.xMin; x < mapBounds.xMax; x++)
        {
            const int tile = terrainMap[get_map_index(x, z)];
            if (tile != Map::GROUND_CUBE && tile != MAZE_WE_DOOR && tile != MAZE_NS_DOOR)
            {
                continue;
            }

            const Vector3 position = {x * T, 0.0f, z * T};

            // TODO: test n/2 instead of a*sqrt(2)/2
            if (!SphereVisible(f, Vector3{position.x, -0.5f * groundScale, position.z}, groundRadius))
                continue;

            if (tile == Map::GROUND_CUBE)
            {
                DrawModel(models.at(ModelId::Ground), position, groundScale, WHITE);
            }
            else if (tile == Map::MAZE_NS_DOOR || tile == Map::MAZE_WE_DOOR)
            {
                DrawModel(models.at(ModelId::Cube), position, 1.0f, PURPLE);
            }

            // collect human data and sell it
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