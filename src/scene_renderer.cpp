#include "scene_renderer.hpp"
#include "rlgl.h"
#include "game_config.hpp"
#include "raymath.h"

#include <math.h>
#include <algorithm>

// ---- frustum cached once per frame ----
struct ViewFrustum
{
    Vector3 pos, fwd, right, up;
    float tanH, tanV, secH, secV, farDist;
};

static ViewFrustum MakeFrustum(const Camera &cam, float farDist)
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
    f.farDist = farDist;
    return f;
}

static bool SphereVisible(const ViewFrustum &f, Vector3 center, float radius)
{
    const Vector3 o = Vector3Subtract(center, f.pos);
    const float depth = Vector3DotProduct(o, f.fwd);

    if (depth + radius < NEAR_PLANE)
        return false;
    if (depth - radius > f.farDist)
        return false; // far plane

    const float h = fabsf(Vector3DotProduct(o, f.right));
    const float v = fabsf(Vector3DotProduct(o, f.up));
    return h <= depth * f.tanH + radius * f.secH &&
           v <= depth * f.tanV + radius * f.secV;
}

// ---- init / unload ----
void SceneRenderer_Init(SceneRenderer &r, Model ground, const Level *level)
{
    const float T = LEVEL_TILE_SIZE;
    const int tiles = 2 * MAP_SIDE_LENGTH;
    const float size = tiles * T;
    const float extent = MAP_SIDE_LENGTH * T;

    // 1) ground: one plane covering the whole map, UVs scaled so the texture repeats once per tile
    r.groundMesh = GenMeshPlane(size, size, 1, 1);
    for (int i = 0; i < r.groundMesh.vertexCount * 2; i++)
        r.groundMesh.texcoords[i] *= (float)tiles;
    UpdateMeshBuffer(r.groundMesh, 1, r.groundMesh.texcoords,
                     r.groundMesh.vertexCount * 2 * (int)sizeof(float), 0);

    r.groundMaterial = ground.materials[ground.meshMaterial[0]]; // borrowed, don't unload it
    SetTextureWrap(r.groundMaterial.maps[MATERIAL_MAP_ALBEDO].texture, TEXTURE_WRAP_REPEAT);

    // old loop placed tile centers at x*T for x in [-N, N), so the covered area is [-N*T - T/2, N*T - T/2)
    r.groundTransform = MatrixTranslate(-0.5f * T, 0.0f, -0.5f * T);

    // 2) walls: precompute transform + bounding sphere, bucket into grid cells
    const int cellsPerSide = (int)ceilf(size / CHUNK_SIZE) + 1;
    r.cells.assign(cellsPerSide * cellsPerSide, WallCell{});

    std::vector<Vector3> cellMin(r.cells.size(), Vector3{1e9f, 1e9f, 1e9f});
    std::vector<Vector3> cellMax(r.cells.size(), Vector3{-1e9f, -1e9f, -1e9f});

    const size_t n = level->walls.size();
    r.wallTransform.resize(n);
    r.wallCenter.resize(n);
    r.wallRadius.resize(n);

    for (size_t i = 0; i < n; i++)
    {
        const WallInstance &w = level->walls[i];
        const float halfLength = WALL_HALF_LENGTH * w.scale;
        const float halfThickness = WALL_HALF_THICKNESS * w.scale;
        const float height = WALL_HEIGHT * w.scale;

        r.wallRadius[i] = sqrtf(halfLength * halfLength + halfThickness * halfThickness + height * height * 0.25f);
        r.wallCenter[i] = {w.position.x, w.position.y + height * 0.5f, w.position.z};

        // same order DrawModelEx uses: scale -> rotate -> translate
        r.wallTransform[i] = MatrixMultiply(
            MatrixMultiply(MatrixScale(w.scale, w.scale, w.scale),
                           MatrixRotate(Vector3{0, 1, 0}, w.rotationDegrees * DEG2RAD)),
            MatrixTranslate(w.position.x, w.position.y, w.position.z));

        int cx = std::clamp((int)((w.position.x + extent) / CHUNK_SIZE), 0, cellsPerSide - 1);
        int cz = std::clamp((int)((w.position.z + extent) / CHUNK_SIZE), 0, cellsPerSide - 1);
        const int ci = cz * cellsPerSide + cx;

        r.cells[ci].walls.push_back((int)i);
        const Vector3 c = r.wallCenter[i];
        const float rad = r.wallRadius[i];
        cellMin[ci] = {fminf(cellMin[ci].x, c.x - rad), fminf(cellMin[ci].y, c.y - rad), fminf(cellMin[ci].z, c.z - rad)};
        cellMax[ci] = {fmaxf(cellMax[ci].x, c.x + rad), fmaxf(cellMax[ci].y, c.y + rad), fmaxf(cellMax[ci].z, c.z + rad)};
    }

    // bounding sphere per cell, drop empty cells
    std::vector<WallCell> nonEmpty;
    for (size_t ci = 0; ci < r.cells.size(); ci++)
    {
        WallCell &cell = r.cells[ci];
        if (cell.walls.empty())
            continue;
        cell.center = Vector3Scale(Vector3Add(cellMin[ci], cellMax[ci]), 0.5f);
        cell.radius = Vector3Length(Vector3Subtract(cellMax[ci], cellMin[ci])) * 0.5f;
        nonEmpty.push_back(std::move(cell));
    }
    r.cells = std::move(nonEmpty);

    r.visibleTransforms.reserve(n);
    r.visibleIdx.reserve(n);
}

void SceneRenderer_Unload(SceneRenderer &r)
{
    UnloadMesh(r.groundMesh); // material is borrowed, the Model owner unloads it
}

// ---- shadow pass: walls only (flat ground receives shadows, it doesn't need to cast them) ----
// The shadow map is baked once, so no distance culling here. If you make it follow the player
// later, add a range test against the wall centers.
void DrawShadowCasters(Model wall, const Level *level)
{
    // for (const WallInstance &w : level->walls)
    // {
    //     DrawModelEx(wall, w.position, Vector3{0, 1, 0}, w.rotationDegrees,
    //                 Vector3{w.scale, w.scale, w.scale}, WHITE);
    // }
}

// ---- main pass ----
RenderStats DrawLevel(SceneRenderer &r, Model wall, const Level *level, const Camera *camera)
{
    RenderStats stats = {};
    const ViewFrustum f = MakeFrustum(*camera, FAR_PLANE);

    // ground: 1 draw call. Drawn as a single instance so it uses the same instancing shader as the walls.
    DrawMeshInstanced(r.groundMesh, r.groundMaterial, &r.groundTransform, 1);
    stats.totalInstances++;
    stats.visibleInstances++;
    stats.drawCalls++;

    // walls: reject whole cells first, then individual walls
    r.visibleTransforms.clear();
    r.visibleIdx.clear();

    for (const WallCell &cell : r.cells)
    {
        stats.totalInstances += (int)cell.walls.size();
        if (!SphereVisible(f, cell.center, cell.radius))
            continue;

        for (int i : cell.walls)
        {
            if (!SphereVisible(f, r.wallCenter[i], r.wallRadius[i]))
                continue;
            r.visibleTransforms.push_back(r.wallTransform[i]);
            r.visibleIdx.push_back(i);
            stats.visibleInstances++;
        }
    }

    // one instanced draw per mesh in the wall model (usually 1)
    if (!r.visibleTransforms.empty())
    {
        for (int m = 0; m < wall.meshCount; m++)
        {
            DrawMeshInstanced(wall.meshes[m], wall.materials[wall.meshMaterial[m]],
                              r.visibleTransforms.data(), (int)r.visibleTransforms.size());
            stats.drawCalls++;
        }
    }

    if (DEBUG_WALL_BOUNDS)
    {
        for (int i : r.visibleIdx)
        {
            const WallInstance &w = level->walls[i];
            const float height = WALL_HEIGHT * w.scale;
            rlPushMatrix();
            rlTranslatef(w.position.x, w.position.y + height * 0.5f, w.position.z);
            rlRotatef(w.rotationDegrees, 0.0f, 1.0f, 0.0f);
            DrawCubeWires({0, 0, 0}, WALL_HALF_LENGTH * w.scale * 2.0f, height,
                          WALL_HALF_THICKNESS * w.scale * 2.0f, RED);
            rlPopMatrix();
        }
    }

    // sun: placed relative to the camera so it never gets clipped by the far plane.
    // Same direction and apparent size as before (300,300,0 with radius 100).
    const Vector3 sunOffset = {300.0f, 300.0f, 0.0f};
    const float sunDist = FAR_PLANE * 0.9f;
    const float sunRadius = 100.0f * sunDist / Vector3Length(sunOffset);
    const Vector3 sunPos = Vector3Add(camera->position, Vector3Scale(Vector3Normalize(sunOffset), sunDist));
    DrawSphere(sunPos, sunRadius, (Color){255, 215, 0, 255});
    stats.totalInstances++;
    stats.visibleInstances++;
    stats.drawCalls++;

    return stats;
}