
#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// physics
#define GRAVITY 32.0f
#define MAX_SPEED 200.0f
#define CROUCH_SPEED 5.0f
#define JUMP_FORCE 55.0f
#define MAX_ACCEL 2000.0f
#define PHYSICS_STEP (1.0f / 120.0f)
#define FRICTION 0.86f
#define AIR_DRAG 0.98f
#define CONTROL 15.0f

// player
#define STAND_HEIGHT 1.0f
#define BOTTOM_HEIGHT 0.5f
#define PLAYER_RADIUS 0.5f
#define CROUCH_HEIGHT 0.0f
#define PLAYER_HEIGHT (BOTTOM_HEIGHT + STAND_HEIGHT)

// rendering
#define CHUNK_SIZE 32.0f
constexpr float LIGHT_NEAR = 1.0f;
constexpr float LIGHT_FAR = 500.0f;

// map
#define MAP_SIDE_LENGTH 325
#define LEVEL_TILE_SIZE 20.0f
#define GROUND_MODEL_SCALE 1.5f

#define WALL_HALF_LENGTH (LEVEL_TILE_SIZE * 0.5f)
#define WALL_HALF_THICKNESS 1.2f // TODO: WRONG
#define WALL_HEIGHT (LEVEL_TILE_SIZE)
#define WALL_MODEL_SCALE 1.5f

// Local-space bounds of resources/wall/maze_wall.obj; used to fit the model
// to the configured collision dimensions.
#define WALL_MODEL_HALF_LENGTH 6.0f
#define WALL_MODEL_HALF_THICKNESS 1.37f
#define WALL_MODEL_MIN_Y (-0.175f)
#define WALL_MODEL_HEIGHT 14.35f

// shaders
#define SHADOW_MAP_SIZE 2048
#define SHADOW_CAMERA_SIZE 160.0f
static const int SHADOW_TEXTURE_SLOT = 10;

// fog
constexpr unsigned char SKY_R = 190;
constexpr unsigned char SKY_G = 205;
constexpr unsigned char SKY_B = 225;
constexpr float FOG_START = 150.0f;
constexpr float FOG_END = 450.0f; // keep this at or below the far plane (500)

// debug
#define DEBUG_WALL_BOUNDS true

#endif
