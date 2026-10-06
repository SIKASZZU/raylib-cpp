#include "shader_system.hpp"

#include "game_config.hpp"
#include "raymath.h"
#include "scene_renderer.hpp"

static void SetModelShader(Model *model, Shader shader);

bool ShaderSystemInitialize(ShaderSystem *system, Model *wall, Model *ground)
{
    *system = {};
    system->lighting = LoadShader("resources/shaders/sun_lighting.vs", "resources/shaders/sun_lighting.fs");
    system->shadow = LoadShader("resources/shaders/shadow_depth.vs", "resources/shaders/shadow_depth.fs");

    if (!IsShaderValid(system->lighting) || !IsShaderValid(system->shadow))
    {
        TraceLog(LOG_ERROR, "Failed to load the lighting or shadow shader");
        ShaderSystemUnload(system);
        return false;
    }

    system->shadowMap = LoadRenderTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    if (!IsRenderTextureValid(system->shadowMap))
    {
        TraceLog(LOG_ERROR, "Failed to create the shadow map render texture");
        ShaderSystemUnload(system);
        return false;
    }

    const Vector3 sunPosition = {300.0f, 300.0f, 0.0f};
    system->lightCamera = {};
    system->lightCamera.position = sunPosition;
    system->lightCamera.target = Vector3{0.0f, 0.0f, 0.0f};
    system->lightCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    system->lightCamera.fovy = SHADOW_CAMERA_SIZE;
    system->lightCamera.projection = CAMERA_ORTHOGRAPHIC;

    SetModelShader(wall, system->shadow);
    SetModelShader(ground, system->shadow);
    BeginTextureMode(system->shadowMap);
    ClearBackground(WHITE);
    BeginMode3D(system->lightCamera);
    BeginShaderMode(system->shadow);
    DrawShadowCasters(*wall, *ground);
    EndShaderMode();
    EndMode3D();
    EndTextureMode();

    Matrix lightProjection = MatrixOrtho(
        -SHADOW_CAMERA_SIZE * 0.5,
        SHADOW_CAMERA_SIZE * 0.5,
        -SHADOW_CAMERA_SIZE * 0.5,
        SHADOW_CAMERA_SIZE * 0.5,
        0.01,
        1000.0);
    Matrix lightViewProjection = MatrixMultiply(GetCameraMatrix(system->lightCamera), lightProjection);
    int lightViewProjectionLocation = GetShaderLocation(system->lighting, "lightViewProj");
    int lightDirectionLocation = GetShaderLocation(system->lighting, "lightDirection");
    int shadowMapLocation = GetShaderLocation(system->lighting, "shadowMap");
    int shadowTexelSizeLocation = GetShaderLocation(system->lighting, "shadowTexelSize");
    Vector3 lightDirection = Vector3Normalize(Vector3Subtract(sunPosition, system->lightCamera.target));
    Vector2 shadowTexelSize = {1.0f / SHADOW_MAP_SIZE, 1.0f / SHADOW_MAP_SIZE};
    SetShaderValueMatrix(system->lighting, lightViewProjectionLocation, lightViewProjection);
    SetShaderValue(system->lighting, lightDirectionLocation, &lightDirection, SHADER_UNIFORM_VEC3);
    SetShaderValueTexture(system->lighting, shadowMapLocation, system->shadowMap.texture);
    SetShaderValue(system->lighting, shadowTexelSizeLocation, &shadowTexelSize, SHADER_UNIFORM_VEC2);

    SetModelShader(wall, system->lighting);
    SetModelShader(ground, system->lighting);
    return true;
}

void ShaderSystemUnload(ShaderSystem *system)
{
    if (IsRenderTextureValid(system->shadowMap))
        UnloadRenderTexture(system->shadowMap);
    if (IsShaderValid(system->shadow))
        UnloadShader(system->shadow);
    if (IsShaderValid(system->lighting))
        UnloadShader(system->lighting);
    *system = {};
}

static void SetModelShader(Model *model, Shader shader)
{
    for (int i = 0; i < model->materialCount; i++)
        model->materials[i].shader = shader;
}
