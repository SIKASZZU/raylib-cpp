#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <iostream>

#include "src/camera_controller.hpp"
#include "src/game_config.hpp"
#include "src/level.hpp"
#include "src/player.hpp"
#include "src/scene_renderer.hpp"
#include "src/model_assets.hpp"
#include "src/robot.hpp"

int main(void)
{
    const int screenWidth = 1440;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "raylib");
    rlSetClipPlanes(LIGHT_NEAR, LIGHT_FAR);

    ModelAssets models = LoadModelAssets();

    Level level = {};
    LevelInitialize(&level);

    Player player = {};
    PlayerInitialize(&player, Vector3{ 0.0f, 0.0f, 0.0f });

    CameraController cameraController = {};
    CameraControllerInitialize(&cameraController, player.position);

    SetTargetFPS(144);

    // random init variables

    // cursor
    DisableCursor();
    SetExitKey(KEY_Q);
    bool cursorCaptured = false;

    // player
    float physicsAccumulator = 0.0f;
    bool jumpQueued = false;

    // scene render
    int showShadowMode = 0;
    bool renderStop = false;
    SceneVector sceneVector;

    Robot robot;
    robot.Init(models);

    while (!WindowShouldClose())
    {
        if (!IsWindowFocused()) {
            if (cursorCaptured) {
                EnableCursor(); cursorCaptured = false;
            }
        }
        else if (!cursorCaptured) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                DisableCursor();
                cursorCaptured = true;
            }
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            EnableCursor();
            cursorCaptured = false;
        }

        if (IsKeyPressed(KEY_F3)) {
            showShadowMode = (showShadowMode + 1) % 3;
        }
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

        if (IsKeyPressed(KEY_F4))
        {
            renderStop = !renderStop;
            if (renderStop)
            {
                sceneVector = Scene::createSceneVector(&level, &camera);
            }
            else
            {
                sceneVector.items.clear();
            }
        }

        robot.Update(delta, player.position);

        // DRAWING

        BeginDrawing();

        ClearBackground(Color{ SKY_R, SKY_G, SKY_B, 255 });

        BeginMode3D(camera);
        Scene::draw(models, &level, &camera, renderStop ? &sceneVector : nullptr);
        robot.Draw();

        EndMode3D();

        Color textColor = GREEN;

        // ON-SCREEN DEBUG TEXT

        // TOP-LEFT
        DrawText("Camera controls:", 15, 15, 10, textColor);
        DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, textColor);
        DrawText("- Look around: arrow keys or mouse", 15, 45, 10, textColor);
        DrawText(TextFormat("- Velocity Len: (%06.3f)", PlayerGetHorizontalSpeed(&player)), 15, 60, 10, textColor);
        DrawText(TextFormat("- Shadow debug (F3): %d", showShadowMode), 15, 75, 10, textColor);
        DrawText(TextFormat("- Shadow debug (F3): %d", showShadowMode), 15, 90, 10, textColor);
        // DrawText(TextFormat("- Draw calls: %d", renderStats.drawCalls), 15, 105, 10, textColor);
        DrawText(TextFormat("- Wall count: %d", level.walls.size()), 15, 120, 10, textColor);
        // DrawText(TextFormat("- In-frustum instances: %d / %d (%d culled)",
        //                     renderStats.visibleInstances, renderStats.totalInstances,
        //                     renderStats.totalInstances - renderStats.visibleInstances),
        //          15, 135, 10, textColor);

        // ON-SCREEN TOP-RIGHT
        DrawText(TextFormat("- xyz: %7.2f %7.2f %7.2f", player.position.x, player.position.y, player.position.z), screenWidth - 350, 0, 20, textColor); // player pos
        DrawText(TextFormat("-grid: %4.0f %4.0f %4.0f", player.position.x / LEVEL_TILE_SIZE, player.position.y / LEVEL_TILE_SIZE, player.position.z / LEVEL_TILE_SIZE), screenWidth - 350, 20, 30, textColor); // player pos

        // ON-SCREEN BOTTOM-LEFT
        DrawFPS(10, screenHeight - 20); // fps

        EndDrawing();
    }

    UnloadModelAssets(models);
    CloseWindow();
    CloseWindow();

    return 0;
}
