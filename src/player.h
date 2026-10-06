#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct
{
    Vector3 position;
    Vector3 velocity;
    Vector3 direction;
    bool isGrounded;
} Player;

void PlayerInitialize(Player *player, Vector3 position);
void PlayerUpdate(Player *player, float rotation, char side, char forward, bool jumpPressed, bool crouchHold, float delta);
float PlayerGetHorizontalSpeed(const Player *player);

#endif
