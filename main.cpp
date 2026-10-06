#include "raylib.h"
#include "raymath.h"

#include "src/camera_controller.hpp"
#include "src/game_config.hpp"
#include "src/level.hpp"
#include "src/player.hpp"
#include "src/scene_renderer.hpp"
#include "src/shader_system.hpp"

int main(void)
{
    const int screenWidth = 1440;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera fps");

    Model wall = LoadModel("resources/wall/maze_wall.obj");
    Model ground = LoadModel("resources/ground/maze_ground.obj");

    Level level = {};
    LevelInitialize(&level);

    ShaderSystem shaders = {};
    if (!ShaderSystemInitialize(&shaders, &wall, &ground, &level))
    {
        UnloadModel(wall);
        UnloadModel(ground);
        CloseWindow();
        return 1;
    }
    Player player = {};
    PlayerInitialize(&player, Vector3{7.0f, 0.0f, 7.0f});

    CameraController cameraController = {};
    CameraControllerInitialize(&cameraController, player.position);

    DisableCursor();
    SetTargetFPS(144);

    float physicsAccumulator = 0.0f;
    bool jumpQueued = false;
    bool showShadowMask = false;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F3))
        {
            showShadowMask = !showShadowMask;
        }

        float delta = GetFrameTime();
        CameraControllerApplyMouse(&cameraController, GetMouseDelta());

        char sideway = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        char forward = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        bool crouching = IsKeyDown(KEY_LEFT_CONTROL);

        physicsAccumulator += delta;
        jumpQueued = jumpQueued || IsKeyPressed(KEY_SPACE);
        while (physicsAccumulator >= PHYSICS_STEP)
        {
            PlayerUpdate(&player, &level, CameraControllerGetYaw(&cameraController),
                         sideway, forward, jumpQueued, crouching, PHYSICS_STEP);
            jumpQueued = false;
            physicsAccumulator -= PHYSICS_STEP;
        }

        CameraControllerUpdate(&cameraController, player.position, player.isGrounded,
                               sideway, forward, crouching, delta);
        Camera camera = CameraControllerGetCamera(&cameraController);

        ShaderSystemUpdateShadowMap(&shaders, &wall, &ground, &level);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        ShaderSystemSetShadowMask(&shaders, showShadowMask);

        BeginMode3D(camera);
        ShaderSystemBeginLighting(&shaders);
        RenderStats renderStats = DrawLevel(wall, ground, &level, &camera);
        ShaderSystemEndLighting();
        EndMode3D();

        DrawRectangle(5, 5, 440, 110, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(5, 5, 440, 110, BLUE);
        DrawText("Camera controls:", 15, 15, 10, BLACK);
        DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
        DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
        DrawText(TextFormat("- Velocity Len: (%06.3f)", PlayerGetHorizontalSpeed(&player)), 15, 60, 10, BLACK);
        DrawText(TextFormat("- Shadow mask (F3): %s", showShadowMask ? "ON" : "OFF"), 15, 75, 10, BLACK);
        DrawText(TextFormat("- In-frustum instances: %d / %d (%d culled)",
                            renderStats.visibleInstances, renderStats.totalInstances,
                            renderStats.totalInstances - renderStats.visibleInstances),
                 15, 90, 10, BLACK);
        DrawFPS(10, screenHeight - 20);
        EndDrawing();
    }

    UnloadModel(wall);
    UnloadModel(ground);
    ShaderSystemUnload(&shaders);
    CloseWindow();

    return 0;
}
