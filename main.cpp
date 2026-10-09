#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "src/camera_controller.hpp"
#include "src/game_config.hpp"
#include "src/level.hpp"
#include "src/player.hpp"
#include "src/scene_renderer.hpp"

int main(void)
{
    const int screenWidth = 1440;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "raylib");
    rlSetClipPlanes(LIGHT_NEAR, LIGHT_FAR);

    Model wall = LoadModel("resources/wall/maze_wall.obj");
    Model ground = LoadModel("resources/ground/maze_ground.obj");

    Level level = {};
    LevelInitialize(&level);

    Player player = {};
    PlayerInitialize(&player, Vector3{7.0f, 0.0f, 7.0f});

    CameraController cameraController = {};
    CameraControllerInitialize(&cameraController, player.position);

    DisableCursor();
    SetTargetFPS(144);

    float physicsAccumulator = 0.0f;
    bool jumpQueued = false;
    int showShadowMode = 0;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F3))
            showShadowMode = (showShadowMode + 1) % 3;

        float delta = GetFrameTime();
        CameraControllerApplyMouse(&cameraController, GetMouseDelta());

        char sideway = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
        char forward = IsKeyDown(KEY_W) - IsKeyDown(KEY_S);
        bool crouching = IsKeyDown(KEY_LEFT_CONTROL);

        physicsAccumulator += delta;
        jumpQueued = jumpQueued || IsKeyPressed(KEY_SPACE);
        bool jumpHolding = IsKeyDown(KEY_SPACE); // TODO: remove these lines if remove god mode
        while (physicsAccumulator >= PHYSICS_STEP)
        {
            PlayerUpdate(&player, &level, CameraControllerGetYaw(&cameraController),
                         sideway, forward, jumpHolding, crouching, PHYSICS_STEP);
            jumpQueued = false;
            physicsAccumulator -= PHYSICS_STEP;
        }

        CameraControllerUpdate(&cameraController, player.position, player.isGrounded,
                               sideway, forward, crouching, delta);
        Camera camera = CameraControllerGetCamera(&cameraController);

        BeginDrawing();
        ClearBackground(Color{SKY_R, SKY_G, SKY_B, 255});

        BeginMode3D(camera);
        RenderStats renderStats = DrawLevel(wall, ground, &level, &camera);
        EndMode3D();

        DrawRectangle(5, 5, 440, 130, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(5, 5, 440, 130, BLUE);

        Color textColor = RED;
        DrawText("Camera controls:", 15, 15, 10, textColor);
        DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, textColor);
        DrawText("- Look around: arrow keys or mouse", 15, 45, 10, textColor);
        DrawText(TextFormat("- Velocity Len: (%06.3f)", PlayerGetHorizontalSpeed(&player)), 15, 60, 10, textColor);
        DrawText(TextFormat("- Shadow debug (F3): %d", showShadowMode), 15, 75, 10, BLACK);
        DrawText(TextFormat("- In-frustum instances: %d / %d (%d culled)",
                            renderStats.visibleInstances, renderStats.totalInstances,
                            renderStats.totalInstances - renderStats.visibleInstances),
                 15, 90, 10, textColor);
        DrawText(TextFormat("- Draw calls: %d", renderStats.drawCalls), 15, 105, 10, textColor);
        DrawText(TextFormat("- Wall count: %d", level.walls.size()), 15, 120, 10, textColor);
        DrawFPS(10, screenHeight - 20); // fps

        EndDrawing();
    }
    UnloadModel(wall);
    UnloadModel(ground);
    CloseWindow();
    CloseWindow();

    return 0;
}
