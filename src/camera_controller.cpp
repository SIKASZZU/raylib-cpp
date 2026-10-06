#include "camera_controller.h"

#include "game_config.h"
#include "raymath.h"

#include <math.h>

static constexpr float MOUSE_SENSITIVITY = 0.001f;
static float GetFrameLerpFactor(float rate, float delta);

void CameraControllerInitialize(CameraController *controller, Vector3 playerPosition)
{
    *controller = {};
    controller->headLerp = STAND_HEIGHT;
    controller->camera.fovy = 60.0f;
    controller->camera.projection = CAMERA_PERSPECTIVE;
    controller->camera.position = {
        playerPosition.x,
        playerPosition.y + BOTTOM_HEIGHT + controller->headLerp,
        playerPosition.z,
    };
    CameraControllerUpdate(controller, playerPosition, false, 0, 0, false, 0.0f);
}

void CameraControllerApplyMouse(CameraController *controller, Vector2 mouseDelta)
{
    controller->rotation.x -= mouseDelta.x * MOUSE_SENSITIVITY;
    controller->rotation.y += mouseDelta.y * MOUSE_SENSITIVITY;
}

void CameraControllerUpdate(CameraController *controller, Vector3 playerPosition, bool isGrounded, char side, char forward, bool crouching, float delta)
{
    controller->headLerp = Lerp(controller->headLerp,
                                crouching ? CROUCH_HEIGHT : STAND_HEIGHT,
                                GetFrameLerpFactor(20.0f, delta));
    controller->camera.position = {
        playerPosition.x,
        playerPosition.y + BOTTOM_HEIGHT + controller->headLerp,
        playerPosition.z,
    };

    if (isGrounded && (forward != 0 || side != 0))
    {
        controller->headTimer += delta * 3.0f;
        controller->walkLerp = Lerp(controller->walkLerp, 1.0f, GetFrameLerpFactor(10.0f, delta));
        controller->camera.fovy = Lerp(controller->camera.fovy, 55.0f, GetFrameLerpFactor(5.0f, delta));
    }
    else
    {
        controller->walkLerp = Lerp(controller->walkLerp, 0.0f, GetFrameLerpFactor(10.0f, delta));
        controller->camera.fovy = Lerp(controller->camera.fovy, 60.0f, GetFrameLerpFactor(5.0f, delta));
    }

    controller->lean.x = Lerp(controller->lean.x, side * 0.02f, GetFrameLerpFactor(10.0f, delta));
    controller->lean.y = Lerp(controller->lean.y, forward * 0.015f, GetFrameLerpFactor(10.0f, delta));

    const Vector3 up = {0.0f, 1.0f, 0.0f};
    const Vector3 targetOffset = {0.0f, 0.0f, -1.0f};
    Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, controller->rotation.x);

    float maxAngleUp = Vector3Angle(up, yaw) - 0.001f;
    if (-controller->rotation.y > maxAngleUp)
        controller->rotation.y = -maxAngleUp;

    float maxAngleDown = -Vector3Angle(Vector3Negate(up), yaw) + 0.001f;
    if (-controller->rotation.y < maxAngleDown)
        controller->rotation.y = -maxAngleDown;

    Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));
    float pitchAngle = Clamp(-controller->rotation.y - controller->lean.y,
                             -PI / 2 + 0.0001f, PI / 2 - 0.0001f);
    Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);

    float headSin = sinf(controller->headTimer * PI);
    float headCos = cosf(controller->headTimer * PI);
    controller->camera.up = Vector3RotateByAxisAngle(up, pitch, headSin * 0.01f + controller->lean.x);

    Vector3 bobbing = Vector3Scale(right, headSin * 0.1f);
    bobbing.y = fabsf(headCos * 0.15f);
    controller->camera.position = Vector3Add(controller->camera.position,
                                             Vector3Scale(bobbing, controller->walkLerp));
    controller->camera.target = Vector3Add(controller->camera.position, pitch);
}

Camera CameraControllerGetCamera(const CameraController *controller)
{
    return controller->camera;
}

float CameraControllerGetYaw(const CameraController *controller)
{
    return controller->rotation.x;
}

static float GetFrameLerpFactor(float rate, float delta)
{
    return 1.0f - expf(-rate * delta);
}
