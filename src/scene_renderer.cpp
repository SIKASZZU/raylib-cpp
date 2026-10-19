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

static ViewFrustum MakeFrustum(const Camera& cam)
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

static bool SphereVisible(const ViewFrustum& f, Vector3 center, float radius)
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

struct WallBounds
{
    BoundingBox visual;   // full mesh incl. joints, used as the thing being tested
    BoundingBox occluder; // inner solid box (the collision proxy), used as the blocker
};

static WallBounds MakeWallBounds(const WallInstance& w)
{
    // Walls are rotated in 90 degree steps, so swapping x/z keeps the AABB exact.
    const bool swapped = fabsf(fmodf(w.rotationDegrees, 180.0f)) > 45.0f;
    const float hx = swapped ? WALL_HALF_THICKNESS : WALL_HALF_LENGTH;
    const float hz = swapped ? WALL_HALF_LENGTH : WALL_HALF_THICKNESS;

    // Joints stick out of the body: 1.37 vs 1.2 in thickness, 0.175 below/above in height (model units).
    const float joint = 1.37f / 1.2f;
    const float vx = swapped ? hx * joint : hx;
    const float vz = swapped ? hz : hz * joint;
    const float vy = 0.175f * WALL_MODEL_SCALE;

    const Vector3 p = w.position;
    WallBounds b;
    b.occluder = { { p.x - hx, p.y, p.z - hz },{ p.x + hx, p.y + WALL_HEIGHT, p.z + hz } };
    b.visual = { { p.x - vx, p.y - vy, p.z - vz },{ p.x + vx, p.y + WALL_HEIGHT + vy, p.z + vz } };
    return b;
}

static bool PointInBox(Vector3 p, const BoundingBox& b)
{
    return p.x >= b.min.x && p.x <= b.max.x &&
        p.y >= b.min.y && p.y <= b.max.y &&
        p.z >= b.min.z && p.z <= b.max.z;
}

static bool IsOccluded(const Vector3& cam, const BoundingBox& candidate, const BoundingBox& occluder)
{
    const Vector3 c[8] = {
        { candidate.min.x, candidate.min.y, candidate.min.z },
        { candidate.max.x, candidate.min.y, candidate.min.z },
        { candidate.min.x, candidate.max.y, candidate.min.z },
        { candidate.max.x, candidate.max.y, candidate.min.z },
        { candidate.min.x, candidate.min.y, candidate.max.z },
        { candidate.max.x, candidate.min.y, candidate.max.z },
        { candidate.min.x, candidate.max.y, candidate.max.z },
        { candidate.max.x, candidate.max.y, candidate.max.z },
    };
    for (const Vector3& p : c)
    {
        const Vector3 d = Vector3Subtract(p, cam);
        const float len = Vector3Length(d);
        if (len < 1e-4f)
            return false;
        const RayCollision hit = GetRayCollisionBox({ cam, Vector3Scale(d, 1.0f / len) }, occluder);
        if (!hit.hit || hit.distance >= len)
            return false; // this corner is visible
    }
    return true;
}

static SceneVector _createSceneVector(const Level* level, const Camera* camera)
{
    SceneVector snapshot;
    const ViewFrustum f = MakeFrustum(*camera);
    const float groundRadius = LEVEL_TILE_SIZE * 0.7071f;
    const MapBoundsVec4 mapBounds = get_map_limits_with_camera(camera);

    for (int z = mapBounds.zMin; z < mapBounds.zMax; z++)
    {
        for (int x = mapBounds.xMin; x < mapBounds.xMax; x++)
        {
            const int tile = terrainMap[get_map_index(x, z)];

            // this if statement is god level in gaining performance
            if (tile == Map::EMPTY)
                continue;

            const Vector3 position = { x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE };
            if (!SphereVisible(f, Vector3{ position.x, -0.5f * GROUND_MODEL_SCALE, position.z }, groundRadius))
                continue;

            snapshot.items.push_back({ tile, position, 0.0f });
        }
    }

    const float halfLength = WALL_HALF_LENGTH;
    const float halfThickness = WALL_HALF_THICKNESS;
    const float height = WALL_HEIGHT;
    const float wallRadius = sqrtf(halfLength * halfLength + halfThickness * halfThickness + height * height * 0.25f);

    // 1. cheap frustum pass
    struct Candidate
    {
        const WallInstance* wall;
        WallBounds bounds;
        float distSq;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(level->walls.size());

    for (const WallInstance& wall : level->walls)
    {
        const Vector3 center = { wall.position.x, wall.position.y + height * 0.5f, wall.position.z };
        if (!SphereVisible(f, center, wallRadius))
            continue;
        candidates.push_back({ &wall, MakeWallBounds(wall), Vector3DistanceSqr(center, camera->position) });
    }

    // 2. nearest first, so the first blockers we test are the most likely ones
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b)
        { return a.distSq < b.distSq; });

    // 3. occlusion pass: only walls nearer than the candidate can block it
    for (size_t i = 0; i < candidates.size(); i++)
    {
        bool occluded = false;
        for (size_t j = 0; j < i && !occluded; j++)
        {
            if (PointInBox(camera->position, candidates[j].bounds.occluder))
                continue; // camera inside this blocker: ray distances are meaningless
            occluded = IsOccluded(camera->position, candidates[i].bounds.visual, candidates[j].bounds.occluder);
        }
        if (!occluded)
            snapshot.items.push_back({ Map::WALL_CUBE, candidates[i].wall->position,
                                      candidates[i].wall->rotationDegrees });
    }

    return snapshot;
}

static void _drawItem(const ModelAssets& models, const SceneItem item)
{
    switch (item.mapTileId)
    {
    case Map::GROUND_CUBE:
        DrawModel(models.at(ModelId::Ground), item.position, GROUND_MODEL_SCALE, WHITE);
        break;

    case Map::MAZE_NS_DOOR:
    case Map::MAZE_WE_DOOR:
        DrawModel(models.at(ModelId::Cube), item.position, 1.0f, GREEN);
        break;

    case Map::WALL_CUBE:
        DrawModelEx(models.at(ModelId::Wall), item.position, Vector3{ 0.0f, 1.0f, 0.0f }, item.rotationDegrees,
            Vector3{ WALL_MODEL_SCALE, WALL_MODEL_SCALE, WALL_MODEL_SCALE }, WHITE);
        break;

    case Map::RED_CUBE:
        DrawModel(models.at(ModelId::Cube), item.position, GROUND_MODEL_SCALE, RED);
        break;

    case Map::BLUE_CUBE:
        DrawModel(models.at(ModelId::Cube), item.position, GROUND_MODEL_SCALE, BLUE);
        break;

    case Map::YELLOW_CUBE:
        DrawModel(models.at(ModelId::Cube), item.position, GROUND_MODEL_SCALE, YELLOW);
        break;

    default:
        DrawModel(models.at(ModelId::Cube), item.position, GROUND_MODEL_SCALE, PURPLE);
        break;
    }
}


void Scene::draw(const ModelAssets& models, const Level* level, const Camera* camera, const SceneVector* sceneItems)
{
    SceneVector currentView;
    if (sceneItems == nullptr)
    {
        currentView = _createSceneVector(level, camera);
        sceneItems = &currentView;
    }

    for (const SceneItem& item : sceneItems->items)
    {
        _drawItem(models, item);
    }
}

SceneVector Scene::createSceneVector(const Level* level, const Camera* camera)
{
    return _createSceneVector(level, camera);
}
