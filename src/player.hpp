#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "level.hpp"

namespace PlayerDimensions
{
inline constexpr float StandHeight = 1.0f;
inline constexpr float BottomHeight = 0.5f;
inline constexpr float Radius = 0.5f;
inline constexpr float CrouchHeight = 0.0f;
inline constexpr float Height = BottomHeight + StandHeight;
}

typedef struct
{
    Vector3 position;
    Vector3 velocity;
    Vector3 direction;
    bool isGrounded;
} Player;

void PlayerInitialize(Player *player, Vector3 position);
void PlayerUpdate(Player *player, const Level *level, float rotation, char side, char forward, bool jumpPressed, bool crouchHold, float delta);
float PlayerGetHorizontalSpeed(const Player *player);

#endif
