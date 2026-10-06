#include "shader_system.hpp"

#include "game_config.hpp"
#include "raymath.h"
#include "rlgl.h"
#include "scene_renderer.hpp"

static void SetModelShader(Model *model, Shader shader);
static RenderTexture2D LoadShadowMapRenderTexture(int width, int height);
static void UnloadShadowMapRenderTexture(RenderTexture2D target);

bool ShaderSystemInitialize(ShaderSystem *system, Model *wall, Model *ground)
{
    *system = {};

    system->lighting = LoadShader("resources/shaders/sun_lighting.vs",
                                  "resources/shaders/sun_lighting.fs");
    system->depth = LoadShader("resources/shaders/shadow_depth.vs",
                               "resources/shaders/shadow_depth.fs");

    if (!IsShaderValid(system->lighting) || !IsShaderValid(system->depth))
    {
        TraceLog(LOG_ERROR, "Failed to load the lighting or shadow-depth shader");
        ShaderSystemUnload(system);
        return false;
    }

    system->shadowMap = LoadShadowMapRenderTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    // A depth-only RenderTexture intentionally has no color texture, so
    // IsRenderTextureValid() would reject it. Validate the FBO + depth texture directly.
    if ((system->shadowMap.id == 0) || !IsTextureValid(system->shadowMap.depth))
    {
        TraceLog(LOG_ERROR, "Failed to create the shadow-map depth texture");
        ShaderSystemUnload(system);
        return false;
    }

    SetTextureFilter(system->shadowMap.depth, TEXTURE_FILTER_POINT);
    SetTextureWrap(system->shadowMap.depth, TEXTURE_WRAP_CLAMP);

    const Vector3 sunPosition = {300.0f, 300.0f, 0.0f};
    system->lightCamera = {};
    system->lightCamera.position = sunPosition;
    system->lightCamera.target = Vector3{0.0f, 0.0f, 0.0f};
    system->lightCamera.up = Vector3{0.0f, 1.0f, 0.0f};
    system->lightCamera.fovy = SHADOW_CAMERA_SIZE;
    system->lightCamera.projection = CAMERA_ORTHOGRAPHIC;

    // PASS 1: Render the static scene from the light into an actual depth texture.
    SetModelShader(wall, system->depth);
    SetModelShader(ground, system->depth);

    Matrix lightView = MatrixIdentity();
    Matrix lightProjection = MatrixIdentity();

    BeginTextureMode(system->shadowMap);
    ClearBackground(WHITE);
    BeginMode3D(system->lightCamera);

    // Capture the exact matrices raylib used for this pass instead of rebuilding them
    // independently and hoping every near/far/aspect convention matches.
    lightView = rlGetMatrixModelview();
    lightProjection = rlGetMatrixProjection();

    DrawShadowCasters(*wall, *ground);

    EndMode3D();
    EndTextureMode();

    const Matrix lightViewProjection = MatrixMultiply(lightView, lightProjection);
    const int lightViewProjectionLocation = GetShaderLocation(system->lighting, "lightViewProj");
    const int lightDirectionLocation = GetShaderLocation(system->lighting, "lightDirection");
    const int shadowTexelSizeLocation = GetShaderLocation(system->lighting, "shadowTexelSize");

    system->shadowMapLocation = GetShaderLocation(system->lighting, "shadowMap");
    system->showShadowMaskLocation = GetShaderLocation(system->lighting, "showShadowMask");

    // Direction from the surface toward the sun.
    const Vector3 lightDirection = Vector3Normalize(Vector3Subtract(sunPosition, system->lightCamera.target));
    const Vector2 shadowTexelSize = {1.0f / (float)SHADOW_MAP_SIZE,
                                     1.0f / (float)SHADOW_MAP_SIZE};
    const int showShadowMask = 0;

    SetShaderValueMatrix(system->lighting, lightViewProjectionLocation, lightViewProjection);
    SetShaderValue(system->lighting, lightDirectionLocation, &lightDirection, SHADER_UNIFORM_VEC3);
    SetShaderValue(system->lighting, shadowTexelSizeLocation, &shadowTexelSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(system->lighting, system->showShadowMaskLocation, &showShadowMask, SHADER_UNIFORM_INT);

    SetModelShader(wall, system->lighting);
    SetModelShader(ground, system->lighting);

    return true;
}

void ShaderSystemBeginLighting(ShaderSystem *system)
{
    // Explicitly bind the shadow map.
    rlEnableShader(system->lighting.id);

    rlActiveTextureSlot(SHADOW_TEXTURE_SLOT);
    rlEnableTexture(system->shadowMap.depth.id);

    rlSetUniform(
        system->shadowMapLocation,
        &SHADOW_TEXTURE_SLOT,
        SHADER_UNIFORM_INT,
        1);
}

void ShaderSystemEndLighting(void)
{
    rlActiveTextureSlot(SHADOW_TEXTURE_SLOT);
    rlDisableTexture();

    rlActiveTextureSlot(0);
    rlDisableShader();
}

void ShaderSystemSetShadowMask(ShaderSystem *system, bool enabled)
{
    const int value = enabled ? 1 : 0;
    SetShaderValue(system->lighting, system->showShadowMaskLocation, &value, SHADER_UNIFORM_INT);
}

void ShaderSystemUnload(ShaderSystem *system)
{
    if (system->shadowMap.id > 0)
        UnloadShadowMapRenderTexture(system->shadowMap);
    if (IsShaderValid(system->depth))
        UnloadShader(system->depth);
    if (IsShaderValid(system->lighting))
        UnloadShader(system->lighting);
    *system = {};
}

static void SetModelShader(Model *model, Shader shader)
{
    for (int i = 0; i < model->materialCount; i++)
        model->materials[i].shader = shader;
}

static RenderTexture2D LoadShadowMapRenderTexture(int width, int height)
{
    RenderTexture2D target = {};
    target.id = rlLoadFramebuffer();

    // BeginTextureMode() uses target.texture dimensions for the viewport even
    // when the framebuffer is depth-only. The color texture id stays zero.
    target.texture.width = width;
    target.texture.height = height;
    target.depth.width = width;
    target.depth.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // false => create a sampleable depth texture, not a renderbuffer.
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.format = 19; // DEPTH_COMPONENT_24BIT in raylib's shadow-map example.
        target.depth.mipmaps = 1;

        rlFramebufferAttach(target.id, target.depth.id,
                            RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        if (!rlFramebufferComplete(target.id))
            TraceLog(LOG_ERROR, "Shadow-map framebuffer is incomplete");

        rlDisableFramebuffer();
    }

    return target;
}

static void UnloadShadowMapRenderTexture(RenderTexture2D target)
{
    if (target.id > 0)
        rlUnloadFramebuffer(target.id);
}
