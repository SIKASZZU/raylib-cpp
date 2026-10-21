

#pragma once


#include <raylib.h>
#include <raymath.h>

#include "model_assets.hpp"


struct Leg {
    Vector3 hipLocal;       // hip anchor in body space
    Vector3 restLocal;      // ideal foot spot in body space
    Vector3 foot;           // current world foot position
    Vector3 stepFrom, stepTo;
    float   stepT = 1.0f;   // 1 = planted
    int     group;          // diagonal pairs: 0 or 1
};

struct Robot {
    float yaw = 0;
    float speed = 2.0f;
    float accel = 1.0f;

    float l1 = 0.8;
    float l2 = 0.8f;
    float stepDist = 1.5f;
    float stepLead = 0.5f;
    float stepSpeed = 3.f;
    float stepHeight = 1.f;

    float scale{ 1.0f };
    float rideHeight = 1.0f;
    float arriveDist = 0.5f;

    Leg legs[4];
    Model body, upper, lower;
    Vector3 pos{ 0, 1, 0 }, vel{ 0, 0, 0 };

    void Init(const ModelAssets& models);
    void Update(float dt, Vector3 target);
    void Draw();

    Matrix BodyMatrix() const {
        return MatrixMultiply(MatrixRotateY(yaw), MatrixTranslate(pos.x, pos.y, pos.z));
    }

    static float GroundY(float, float) { return 0.0f; }  // replace if terrain isn't flat

    static float AngleLerp(float a, float b, float t) {
        float d = fmodf(b - a + PI, 2 * PI);
        if (d < 0) d += 2 * PI;
        return a + (d - PI) * t;
    }

    // Two-bone IK: returns knee position and clamps foot to reachable range
    Vector3 SolveKnee(Vector3 hip, Vector3& foot, Vector3 pole) const {
        Vector3 d = Vector3Subtract(foot, hip);
        float dist = Clamp(Vector3Length(d), 0.001f, l1 + l2 - 0.001f);
        Vector3 dir = Vector3Normalize(d);
        foot = Vector3Add(hip, Vector3Scale(dir, dist));

        float a = (l1 * l1 - l2 * l2 + dist * dist) / (2.0f * dist);
        float h = sqrtf(fmaxf(l1 * l1 - a * a, 0.0f));
        Vector3 side = Vector3Subtract(pole, Vector3Scale(dir, Vector3DotProduct(pole, dir)));
        side = Vector3Normalize(side);
        return Vector3Add(Vector3Add(hip, Vector3Scale(dir, a)), Vector3Scale(side, h));
    }

    void DrawSegment(Model& m, Vector3 from, Vector3 to) {
        Vector3 dir = Vector3Normalize(Vector3Subtract(to, from));
        Quaternion q = QuaternionFromVector3ToVector3({ 0, 1, 0 }, dir);
        m.transform = MatrixMultiply(
            MatrixMultiply(MatrixScale(scale, scale, scale), QuaternionToMatrix(q)),
            MatrixTranslate(from.x, from.y, from.z));
        DrawModel(m, Vector3Zero(), 1.0f, WHITE);
    }
};