#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "raylib.h"

typedef struct
{
    Camera camera;
    Vector2 rotation;
    float headTimer;
    float walkLerp;
    float headLerp;
    Vector2 lean;
} CameraController;

void CameraControllerInitialize(CameraController *controller, Vector3 playerPosition);
void CameraControllerApplyMouse(CameraController *controller, Vector2 mouseDelta);
void CameraControllerUpdate(CameraController *controller, Vector3 playerPosition, bool isGrounded, char side, char forward, bool crouching, float delta);
Camera CameraControllerGetCamera(const CameraController *controller);
float CameraControllerGetYaw(const CameraController *controller);

#endif
