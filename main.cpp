
#include "raylib.h"
#include "raymath.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Movement constants
#define GRAVITY 32.0f
#define MAX_SPEED 20.0f
#define CROUCH_SPEED 5.0f
#define JUMP_FORCE 55.0f
#define MAX_ACCEL 2000.0f
#define PHYSICS_STEP (1.0f / 120.0f)
// Grounded drag
#define FRICTION 0.86f
// Increasing air drag, increases strafing speed
#define AIR_DRAG 0.98f
// Responsiveness for turning movement direction to looked direction
#define CONTROL 15.0f
#define CROUCH_HEIGHT 0.0f

// player
#define STAND_HEIGHT 1.0f
#define BOTTOM_HEIGHT 0.5f
#define PLAYER_RADIUS 0.5f
#define PLAYER_HEIGHT (BOTTOM_HEIGHT + STAND_HEIGHT)

#define WALL_HALF_LENGTH 6.0f
#define WALL_HALF_THICKNESS 1.2f
#define WALL_HEIGHT 14.0f
#define ROTATED_WALL_ANGLE 45.0f
#define NORMAL_WALL_SCALE 1.5f
#define LEVEL_TILE_SIZE 20.0f
#define GROUND_MODEL_SIZE 12.0f

#define NORMALIZE_INPUT 1

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Body structure
typedef struct
{
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool isGrounded;
} Body;

typedef struct
{
    int totalInstances;
    int visibleInstances;
} RenderStats;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static Vector2 sensitivity = {0.001f, 0.001f};

static Body player = {0};
static Vector2 lookRotation = {0};
static float headTimer = 0.0f;
static float walkLerp = 0.0f;
static float headLerp = STAND_HEIGHT;
static Vector2 lean = {0};

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static RenderStats DrawLevel(Model wall, Model ground, const Camera *camera);
static bool IsSphereInCameraView(const Camera *camera, Vector3 center, float radius);
static void UpdateCameraFPS(Camera *camera);
static void UpdateBody(Body *body, float rot, char side, char forward, bool jumpPressed, bool crouchHold, float delta);
static bool CheckPlayerCollision(Vector3 position);
static bool CheckPlayerAgainstWall(Vector3 position, Vector3 wallPosition, float halfLength, float halfThickness, float height, float rotation);
static float GetFrameLerpFactor(float rate, float delta);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

int main(void)
{

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1440;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera fps");

    // Model model = LoadModel("resources/models/obj/castle.obj");                 // Load model
    // Texture2D texture = LoadTexture("resources/models/obj/castle_diffuse.png"); // Load model texture
    // model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;            // Set map diffuse texture

    Model wall = LoadModel("resources/wall/maze_wall.obj");
    Model ground = LoadModel("resources/ground/maze_ground.obj");

    player.position = (Vector3){7.0f, 0.0f, 7.0f};

    // Initialize camera variables
    // NOTE: UpdateCameraFPS() takes care of the rest
    Camera camera = {0};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position = (Vector3){
        player.position.x,
        player.position.y + (BOTTOM_HEIGHT + headLerp),
        player.position.z,
    };

    UpdateCameraFPS(&camera); // Update camera parameters

    DisableCursor(); // Limit cursor to relative movement inside the window
    SetTargetFPS(0);
    float physicsAccumulator = 0.0f;
    bool jumpQueued = false;
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector2 mouseDelta = GetMouseDelta();
        lookRotation.x -= mouseDelta.x * sensitivity.x;
        lookRotation.y += mouseDelta.y * sensitivity.y;

        char sideway = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
        char forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
        bool crouching = IsKeyDown(KEY_LEFT_CONTROL);

        float delta = GetFrameTime();
        physicsAccumulator += delta;
        jumpQueued = jumpQueued || IsKeyPressed(KEY_SPACE);
        while (physicsAccumulator >= PHYSICS_STEP)
        {
            UpdateBody(&player, lookRotation.x, sideway, forward, jumpQueued, crouching, PHYSICS_STEP);
            jumpQueued = false;
            physicsAccumulator -= PHYSICS_STEP;
        }

        headLerp = Lerp(headLerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT), GetFrameLerpFactor(20.0f, delta));
        camera.position = (Vector3){
            player.position.x,
            player.position.y + (BOTTOM_HEIGHT + headLerp),
            player.position.z,
        };

        if (player.isGrounded && ((forward != 0) || (sideway != 0)))
        {
            headTimer += delta * 3.0f;
            walkLerp = Lerp(walkLerp, 1.0f, GetFrameLerpFactor(10.0f, delta));
            camera.fovy = Lerp(camera.fovy, 55.0f, GetFrameLerpFactor(5.0f, delta));
        }
        else
        {
            walkLerp = Lerp(walkLerp, 0.0f, GetFrameLerpFactor(10.0f, delta));
            camera.fovy = Lerp(camera.fovy, 60.0f, GetFrameLerpFactor(5.0f, delta));
        }

        lean.x = Lerp(lean.x, sideway * 0.02f, GetFrameLerpFactor(10.0f, delta));
        lean.y = Lerp(lean.y, forward * 0.015f, GetFrameLerpFactor(10.0f, delta));

        UpdateCameraFPS(&camera);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        RenderStats renderStats = {0};
        BeginMode3D(camera);
        renderStats = DrawLevel(wall, ground, &camera);
        EndMode3D();

        // Draw info box
        DrawRectangle(5, 5, 380, 95, Fade(SKYBLUE, 0.5f));
        DrawRectangleLines(5, 5, 380, 95, BLUE);

        DrawText("Camera controls:", 15, 15, 10, BLACK);
        DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
        DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
        DrawText(TextFormat("- Velocity Len: (%06.3f)", Vector2Length((Vector2){player.velocity.x, player.velocity.z})), 15, 60, 10, BLACK);
        DrawText(TextFormat("- In-frustum instances: %d / %d (%d culled)", renderStats.visibleInstances, renderStats.totalInstances, renderStats.totalInstances - renderStats.visibleInstances), 15, 75, 10, BLACK);
        DrawFPS(10, screenHeight - 20);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    UnloadModel(wall);
    UnloadModel(ground);

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Update body considering current world state
void UpdateBody(Body *body, float rot, char side, char forward, bool jumpPressed, bool crouchHold, float delta)
{
    Vector2 input = (Vector2){(float)side, (float)-forward};

#if defined(NORMALIZE_INPUT)
    // Slow down diagonal movement
    if ((side != 0) && (forward != 0))
        input = Vector2Normalize(input);
#endif

    if (!body->isGrounded)
        body->velocity.y -= GRAVITY * delta;

    if (body->isGrounded && jumpPressed)
    {
        body->velocity.y = JUMP_FORCE;
        body->isGrounded = false;

        // Sound can be played at this moment
        // SetSoundPitch(fxJump, 1.0f + (GetRandomValue(-100, 100)*0.001));
        // PlaySound(fxJump);
    }

    Vector3 front = (Vector3){sinf(rot), 0.f, cosf(rot)};
    Vector3 right = (Vector3){cosf(-rot), 0.f, sinf(-rot)};

    Vector3 desiredDir = (Vector3){
        input.x * right.x + input.y * front.x,
        0.0f,
        input.x * right.z + input.y * front.z,
    };
    float controlFactor = 1.0f - powf(1.0f - CONTROL / 60.0f, delta * 60.0f);
    body->dir = Vector3Lerp(body->dir, desiredDir, controlFactor);

    float decel = powf(body->isGrounded ? FRICTION : AIR_DRAG, delta * 60.0f);
    Vector3 hvel = (Vector3){body->velocity.x * decel, 0.0f, body->velocity.z * decel};

    float hvelLength = Vector3Length(hvel); // Magnitude
    if (hvelLength < (MAX_SPEED * 0.01f))
        hvel = (Vector3){0};

    // This is what creates strafing
    float speed = Vector3DotProduct(hvel, body->dir);

    // Whenever the amount of acceleration to add is clamped by the maximum acceleration constant,
    // a Player can make the speed faster by bringing the direction closer to horizontal velocity angle
    // More info here: https://youtu.be/v3zT3Z5apaM?t=165
    float maxSpeed = (crouchHold ? CROUCH_SPEED : MAX_SPEED);
    float accel = Clamp(maxSpeed - speed, 0.f, MAX_ACCEL * delta);
    hvel.x += body->dir.x * accel;
    hvel.z += body->dir.z * accel;

    body->velocity.x = hvel.x;
    body->velocity.z = hvel.z;

    body->position.y += body->velocity.y * delta;

    Vector3 nextPosition = body->position;
    nextPosition.x += body->velocity.x * delta;
    if (CheckPlayerCollision(nextPosition))
        body->velocity.x = 0.0f;
    else
        body->position.x = nextPosition.x;

    nextPosition = body->position;
    nextPosition.z += body->velocity.z * delta;
    if (CheckPlayerCollision(nextPosition))
        body->velocity.z = 0.0f;
    else
        body->position.z = nextPosition.z;

    // Fancy collision system against the floor
    if (body->position.y <= 0.0f)
    {
        body->position.y = 0.0f;
        body->velocity.y = 0.0f;
        body->isGrounded = true; // Enable jumping
    }

    // clamp version 1
    Vector2 hvelClamping = {body->velocity.x, body->velocity.z};
    if (Vector2Length(hvelClamping) > MAX_SPEED)
    {
        hvelClamping = Vector2Scale(Vector2Normalize(hvelClamping), MAX_SPEED);
        body->velocity.x = hvelClamping.x;
        body->velocity.z = hvelClamping.y;
    }

    // clamp version 2
    if (Vector3Length(body->velocity) > MAX_SPEED)
    {
        body->velocity = Vector3Scale(Vector3Normalize(body->velocity), MAX_SPEED);
    }
}

static float GetFrameLerpFactor(float rate, float delta)
{
    return 1.0f - expf(-rate * delta);
}

static bool CheckPlayerCollision(Vector3 position)
{
    const int floorExtent = 10;

    for (int z = -floorExtent; z < floorExtent; z++)
    {
        for (int x = -floorExtent; x < floorExtent; x++)
        {
            Vector3 wallPosition = {x * LEVEL_TILE_SIZE, 0.0f, z * LEVEL_TILE_SIZE};

            if ((z & 1) && (x & 1))
            {
                if (CheckPlayerAgainstWall(position, wallPosition, WALL_HALF_LENGTH, WALL_HALF_THICKNESS, WALL_HEIGHT, ROTATED_WALL_ANGLE * DEG2RAD))
                    return true;
            }
            else if (!(z & 1) && !(x & 1))
            {
                if (CheckPlayerAgainstWall(position, wallPosition, WALL_HALF_LENGTH * NORMAL_WALL_SCALE, WALL_HALF_THICKNESS * NORMAL_WALL_SCALE, WALL_HEIGHT * NORMAL_WALL_SCALE, 0.0f))
                    return true;
            }
        }
    }

    BoundingBox playerBounds = {
        (Vector3){position.x - PLAYER_RADIUS, position.y, position.z - PLAYER_RADIUS},
        (Vector3){position.x + PLAYER_RADIUS, position.y + PLAYER_HEIGHT, position.z + PLAYER_RADIUS},
    };
    const Vector3 towerSize = {16.0f, 32.0f, 16.0f};
    for (int xSign = -1; xSign <= 1; xSign += 2)
    {
        for (int zSign = -1; zSign <= 1; zSign += 2)
        {
            Vector3 towerPosition = {16.0f * xSign, 16.0f, 16.0f * zSign};
            BoundingBox towerBounds = {
                (Vector3){towerPosition.x - towerSize.x * 0.5f, towerPosition.y - towerSize.y * 0.5f, towerPosition.z - towerSize.z * 0.5f},
                (Vector3){towerPosition.x + towerSize.x * 0.5f, towerPosition.y + towerSize.y * 0.5f, towerPosition.z + towerSize.z * 0.5f},
            };
            if (CheckCollisionBoxes(playerBounds, towerBounds))
                return true;
        }
    }

    return false;
}

static bool CheckPlayerAgainstWall(Vector3 position, Vector3 wallPosition, float halfLength, float halfThickness, float height, float rotation)
{
    if (position.y > height || position.y + PLAYER_HEIGHT < 0.0f)
        return false;

    float cosRotation = cosf(rotation);
    float sinRotation = sinf(rotation);
    float offsetX = position.x - wallPosition.x;
    float offsetZ = position.z - wallPosition.z;
    float localX = cosRotation * offsetX - sinRotation * offsetZ;
    float localZ = sinRotation * offsetX + cosRotation * offsetZ;
    float closestX = Clamp(localX, -halfLength, halfLength);
    float closestZ = Clamp(localZ, -halfThickness, halfThickness);
    float deltaX = localX - closestX;
    float deltaZ = localZ - closestZ;

    return deltaX * deltaX + deltaZ * deltaZ <= PLAYER_RADIUS * PLAYER_RADIUS;
}

// Update camera for FPS behaviour
static void UpdateCameraFPS(Camera *camera)
{
    const Vector3 up = (Vector3){0.0f, 1.0f, 0.0f};
    const Vector3 targetOffset = (Vector3){0.0f, 0.0f, -1.0f};

    // Left and right
    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, lookRotation.x);

    // Clamp view up
    float maxAngleUp = Vector3Angle(up, yaw);
    maxAngleUp -= 0.001f; // Avoid numerical errors
    if (-(lookRotation.y) > maxAngleUp)
    {
        lookRotation.y = -maxAngleUp;
    }

    // Clamp view down
    float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
    maxAngleDown *= -1.0f;  // Downwards angle is negative
    maxAngleDown += 0.001f; // Avoid numerical errors
    if (-(lookRotation.y) < maxAngleDown)
    {
        lookRotation.y = -maxAngleDown;
    }

    // Up and down
    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

    // Rotate view vector around right axis
    float pitchAngle = -lookRotation.y - lean.y;
    pitchAngle = Clamp(pitchAngle, -PI / 2 + 0.0001f, PI / 2 - 0.0001f); // Clamp angle so it doesn't go past straight up or straight down
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);

    // Head animation
    // Rotate up direction around forward axis
    float headSin = sinf(headTimer * PI);
    float headCos = cosf(headTimer * PI);
    const float stepRotation = 0.01f;
    camera->up = Vector3RotateByAxisAngle(up, pitch, headSin * stepRotation + lean.x);

    // Camera BOB
    const float bobSide = 0.1f;
    const float bobUp = 0.15f;
    Vector3 bobbing = Vector3Scale(right, headSin * bobSide);
    bobbing.y = fabsf(headCos * bobUp);

    camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));
    camera->target = Vector3Add(camera->position, pitch);
}

static bool IsSphereInCameraView(const Camera *camera, Vector3 center, float radius)
{
    const Vector3 forward = Vector3Normalize(Vector3Subtract(camera->target, camera->position));
    const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera->up));
    const Vector3 cameraUp = Vector3Normalize(Vector3CrossProduct(right, forward));
    const Vector3 offset = Vector3Subtract(center, camera->position);
    const float depth = Vector3DotProduct(offset, forward);

    if (depth + radius < 0.01f)
        return false;

    const float halfVerticalFov = camera->fovy * DEG2RAD * 0.5f;
    const float tanVertical = tanf(halfVerticalFov);
    const float tanHorizontal = tanVertical * ((float)GetScreenWidth() / GetScreenHeight());
    const float horizontal = fabsf(Vector3DotProduct(offset, right));
    const float vertical = fabsf(Vector3DotProduct(offset, cameraUp));

    if (horizontal > depth * tanHorizontal + radius * sqrtf(1.0f + tanHorizontal * tanHorizontal))
        return false;
    if (vertical > depth * tanVertical + radius * sqrtf(1.0f + tanVertical * tanVertical))
        return false;

    return true;
}

// Draw only scene instances whose bounding spheres intersect the camera view.
static RenderStats DrawLevel(Model wall, Model ground, const Camera *camera)
{
    const int floorExtent = 10;
    const float groundScale = LEVEL_TILE_SIZE / GROUND_MODEL_SIZE;
    const float groundRadius = sqrtf(2.0f * LEVEL_TILE_SIZE * LEVEL_TILE_SIZE * 0.25f +
                                     0.25f * groundScale * groundScale);
    const float wallRadiusBase = sqrtf(WALL_HALF_LENGTH * WALL_HALF_LENGTH +
                                       (WALL_HEIGHT * 0.5f + 0.2f) * (WALL_HEIGHT * 0.5f + 0.2f) +
                                       1.4f * 1.4f);
    RenderStats stats = {0};
    // Floor tiles
    for (int y = -floorExtent; y < floorExtent; y++)
    {
        for (int x = -floorExtent; x < floorExtent; x++)
        {
            Vector3 position = {x * LEVEL_TILE_SIZE, 0.0f, y * LEVEL_TILE_SIZE};
            stats.totalInstances++;
            if (IsSphereInCameraView(camera, (Vector3){position.x, -0.5f * groundScale, position.z}, groundRadius))
            {
                DrawModel(ground, position, groundScale, WHITE);
                stats.visibleInstances++;
            }

            if ((y & 1) && (x & 1))
            {
                const float wallRadius = wallRadiusBase;
                stats.totalInstances++;
                if (IsSphereInCameraView(camera, (Vector3){position.x, WALL_HEIGHT * 0.5f, position.z}, wallRadius))
                {
                    DrawModelEx(
                        wall,
                        position,
                        (Vector3){0.0f, 1.0f, 0.0f}, // rotate around Y
                        ROTATED_WALL_ANGLE,          // degrees
                        (Vector3){1.0f, 1.0f, 1.0f}, // LIGHTGRAY
                        RED);
                    stats.visibleInstances++;
                }
            }
            else if (!(y & 1) && !(x & 1))
            {
                const float wallScale = NORMAL_WALL_SCALE;
                const float wallRadius = wallRadiusBase * wallScale;
                stats.totalInstances++;
                if (IsSphereInCameraView(camera,
                                         (Vector3){position.x, WALL_HEIGHT * wallScale * 0.5f, position.z},
                                         wallRadius))
                {
                    DrawModel(wall, position, wallScale, WHITE);
                    stats.visibleInstances++;
                }
            }
        }
    }

    const Vector3 towerSize = (Vector3){16.0f, 32.0f, 16.0f};
    const Color towerColor = (Color){150, 200, 200, 255};
    const float towerRadius = sqrtf(8.0f * 8.0f + 16.0f * 16.0f + 8.0f * 8.0f);

    for (int xSign = -1; xSign <= 1; xSign += 2)
    {
        for (int zSign = -1; zSign <= 1; zSign += 2)
        {
            Vector3 towerPos = {16.0f * xSign, 16.0f, 16.0f * zSign};
            stats.totalInstances++;
            if (IsSphereInCameraView(camera, towerPos, towerRadius))
            {
                DrawCubeV(towerPos, towerSize, towerColor);
                DrawCubeWiresV(towerPos, towerSize, DARKBLUE);
                stats.visibleInstances++;
            }
        }
    }

    // Yellow sun
    const Vector3 sunPosition = {300.0f, 300.0f, 0.0f};
    stats.totalInstances++;
    if (IsSphereInCameraView(camera, sunPosition, 100.0f))
    {
        DrawSphere(sunPosition, 100.0f, (Color){255, 215, 0, 255});
        stats.visibleInstances++;
    }

    return stats;
}