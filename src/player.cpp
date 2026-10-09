#include "player.hpp"

#include "game_config.hpp"
#include "raymath.h"

#include <math.h>

static bool CheckCollision(Vector3 position, const Level *level);
static bool CheckAgainstWall(Vector3 position, const WallInstance &wall);

void PlayerInitialize(Player *player, Vector3 position)
{
    *player = {};
    player->position = position;
}

void PlayerUpdate(Player *player, const Level *level, float rotation, char side, char forward, bool jumpPressed, bool crouchHold, float delta)
{
    Vector2 input = {(float)side, (float)-forward};
    if (side != 0 && forward != 0)
        input = Vector2Normalize(input);

    // if (!player->isGrounded)
    //     player->velocity.y -= GRAVITY * delta;

    // if (player->isGrounded && jumpPressed)
    // {
    //     player->velocity.y = JUMP_FORCE;
    //     player->isGrounded = false;
    // }

    Vector3 front = {sinf(rotation), 0.0f, cosf(rotation)};
    Vector3 right = {cosf(-rotation), 0.0f, sinf(-rotation)};
    Vector3 desiredDirection = {
        input.x * right.x + input.y * front.x,
        0.0f,
        input.x * right.z + input.y * front.z,
    };
    float controlFactor = 1.0f - powf(1.0f - CONTROL / 60.0f, delta * 60.0f);
    player->direction = Vector3Lerp(player->direction, desiredDirection, controlFactor);

    float decel = powf(player->isGrounded ? FRICTION : AIR_DRAG, delta * 60.0f);
    Vector3 horizontalVelocity = {player->velocity.x * decel, 0.0f, player->velocity.z * decel};
    if (Vector3Length(horizontalVelocity) < MAX_SPEED * 0.01f)
        horizontalVelocity = {};

    float speed = Vector3DotProduct(horizontalVelocity, player->direction);
    float maxSpeed = crouchHold ? CROUCH_SPEED : MAX_SPEED;
    maxSpeed = MAX_SPEED; // TODO: remove these lines if remove god mode
    float acceleration = Clamp(maxSpeed - speed, 0.0f, MAX_ACCEL * delta);
    horizontalVelocity.x += player->direction.x * acceleration;
    horizontalVelocity.z += player->direction.z * acceleration;

    player->velocity.x = horizontalVelocity.x;
    player->velocity.z = horizontalVelocity.z;
    player->position.y += player->velocity.y * delta;

    player->position.x += player->velocity.x * delta; // TODO: remove these lines if remove god mode
    player->position.z += player->velocity.z * delta; // TODO: remove these lines if remove god mode

    // TODO: remove these lines if remove god mode
    if (jumpPressed) {
        player->position.y += acceleration / GRAVITY;
    }
    // TODO: remove these lines if remove god mode
    if (crouchHold) {
        player->position.y -= acceleration / GRAVITY;
    }

    // Vector3 nextPosition = player->position;
    // nextPosition.x += player->velocity.x * delta;
    // if (CheckCollision(nextPosition, level))
    //     player->velocity.x = 0.0f;
    // else
    //     player->position.x = nextPosition.x;

    // nextPosition = player->position;
    // nextPosition.z += player->velocity.z * delta;
    // if (CheckCollision(nextPosition, level))
    //     player->velocity.z = 0.0f;
    // else
    //     player->position.z = nextPosition.z;

    // if (player->position.y <= 0.0f)
    // {
    //     player->position.y = 0.0f;
    //     player->velocity.y = 0.0f;
    //     player->isGrounded = true;
    // }

    // Vector2 horizontalClamped = {player->velocity.x, player->velocity.z};
    // if (Vector2Length(horizontalClamped) > MAX_SPEED)
    // {
    //     horizontalClamped = Vector2Scale(Vector2Normalize(horizontalClamped), MAX_SPEED);
    //     player->velocity.x = horizontalClamped.x;
    //     player->velocity.z = horizontalClamped.y;
    // }

    if (Vector3Length(player->velocity) > MAX_SPEED)
        player->velocity = Vector3Scale(Vector3Normalize(player->velocity), MAX_SPEED);
}

float PlayerGetHorizontalSpeed(const Player *player)
{
    return Vector2Length(Vector2{player->velocity.x, player->velocity.z});
}

static bool CheckCollision(Vector3 position, const Level *level)
{
    for (const WallInstance &wall : level->walls)
    {
        if (CheckAgainstWall(position, wall))
            return true;
    }

    return false;
}

static bool CheckAgainstWall(Vector3 position, const WallInstance &wall)
{
    const float height = WALL_HEIGHT;
    const float halfLength = WALL_HALF_LENGTH;
    const float halfThickness = WALL_HALF_THICKNESS;
    const float rotation = wall.rotationDegrees * DEG2RAD;

    if (position.y > wall.position.y + height || position.y + PLAYER_HEIGHT < wall.position.y)
        return false;

    float cosRotation = cosf(rotation);
    float sinRotation = sinf(rotation);
    float offsetX = position.x - wall.position.x;
    float offsetZ = position.z - wall.position.z;
    float localX = cosRotation * offsetX - sinRotation * offsetZ;
    float localZ = sinRotation * offsetX + cosRotation * offsetZ;
    float closestX = Clamp(localX, -halfLength, halfLength);
    float closestZ = Clamp(localZ, -halfThickness, halfThickness);
    float deltaX = localX - closestX;
    float deltaZ = localZ - closestZ;
    return deltaX * deltaX + deltaZ * deltaZ <= PLAYER_RADIUS * PLAYER_RADIUS;
}
