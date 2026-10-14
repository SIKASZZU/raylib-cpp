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

static RenderSnapshot CollectVisibleItems(const Level *level, const Camera *camera)
{
    RenderSnapshot snapshot;
    const ViewFrustum f = MakeFrustum(*camera);
    const float groundRadius = LEVEL_TILE_SIZE * 0.7071f;
    const MapBoundsVec4 mapBounds = get_map_limits_with_camera(camera);

    for (int z = mapBounds.zMin; z < mapBounds.zMax; z++)
    {
        for (int x = mapBounds.xMin; x < mapBounds.xMax; x++)
        {
            const int tile = terrainMap[get_map_index(x, z)];
            if (tile != Map::GROUND_CUBE && tile != Map::MAZE_WE_DOOR && tile != Map::MAZE_NS_DOOR)
                continue;

            const Vector3 position = {x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE};
            if (!SphereVisible(f, Vector3{position.x, -0.5f * GROUND_MODEL_SCALE, position.z}, groundRadius))
                continue;

            const RenderItemType type = tile == Map::GROUND_CUBE ? RenderItemType::Ground : RenderItemType::Door;
            snapshot.items.push_back({type, position, 0.0f});
        }
    }

    const float halfLength = WALL_HALF_LENGTH;
    const float halfThickness = WALL_HALF_THICKNESS;
    const float height = WALL_HEIGHT;
    const float wallRadius = sqrtf(halfLength * halfLength + halfThickness * halfThickness + height * height * 0.25f);

    for (const WallInstance &wall : level->walls)
    {
        const Vector3 center = {wall.position.x, wall.position.y + height * 0.5f, wall.position.z};
        if (SphereVisible(f, center, wallRadius))
            snapshot.items.push_back({RenderItemType::Wall, wall.position, wall.rotationDegrees});
    }

    return snapshot;
}

RenderSnapshot CaptureRenderSnapshot(const Level *level, const Camera *camera)
{
    return CollectVisibleItems(level, camera);
}

static void DrawRenderItem(const ModelAssets &models, const RenderItem &item)
{
    switch (item.type)
    {
    case RenderItemType::Ground:
        DrawModel(models.at(ModelId::Ground), item.position, GROUND_MODEL_SCALE, WHITE);
        break;
    case RenderItemType::Door:
        DrawModel(models.at(ModelId::Cube), item.position, 1.0f, PURPLE);
        break;
    case RenderItemType::Wall:
        DrawModelEx(models.at(ModelId::Wall), item.position, Vector3{0.0f, 1.0f, 0.0f}, item.rotationDegrees,
                    Vector3{WALL_MODEL_SCALE, WALL_MODEL_SCALE, WALL_MODEL_SCALE}, WHITE);
        break;
    }
}

RenderStats DrawLevel(const ModelAssets &models, const Level *level, const Camera *camera,
                      const RenderSnapshot *renderSnapshot)
{
    RenderStats stats = {};
    RenderSnapshot currentView;
    if (renderSnapshot == nullptr)
    {
        currentView = CollectVisibleItems(level, camera);
        renderSnapshot = &currentView;
    }

    stats.totalInstances = 4 * MAP_SIDE_LENGTH * MAP_SIDE_LENGTH + static_cast<int>(level->walls.size());
    for (const RenderItem &item : renderSnapshot->items)
    {
        DrawRenderItem(models, item);
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