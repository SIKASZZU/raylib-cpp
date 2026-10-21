

#pragma once


#include <raylib.h>
#include <raymath.h>

#include "model_assets.hpp"
// #include "asterion.hpp"


struct Leg {
    Vector3 hipLocal;       // hip anchor in body space
    Vector3 restLocal;      // ideal foot spot in body space
    Vector3 foot;           // current world foot position
    Vector3 stepFrom, stepTo;
    float   stepT = 1.0f;   // 1 = planted
    int     group;          // diagonal pairs: 0 or 1
};

struct Walker {
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

    void Init(const ModelAssets& models) {
        // init body parts
        body = models.at(ModelId::AsterionBody);
        upper = models.at(ModelId::AsterionUpper);
        lower = models.at(ModelId::AsterionLower);

        l1 *= scale; l2 *= scale;
        speed *= scale;
        stepDist *= scale; stepHeight *= scale;
        rideHeight *= scale; arriveDist *= scale;
        pos.y = rideHeight;

        float x = 0.5f * scale, z = 0.6f * scale;   // hip and rest positions scale too
        // FL, FR, BL, BR. Diagonals share a group.
        legs[0] = { { -x,0, z },{ -x * 1.8f,0, z * 1.5f },{},{},{}, 1.0f, 0 };
        legs[1] = { { x,0, z },{ x * 1.8f,0, z * 1.5f },{},{},{}, 1.0f, 1 };
        legs[2] = { { -x,0,-z },{ -x * 1.8f,0,-z * 1.5f },{},{},{}, 1.0f, 1 };
        legs[3] = { { x,0,-z },{ x * 1.8f,0,-z * 1.5f },{},{},{}, 1.0f, 0 };
        Matrix m = BodyMatrix();
        for (auto& l : legs) {
            l.foot = Vector3Transform(l.restLocal, m);
            l.foot.y = GroundY(l.foot.x, l.foot.z);
        }

    }

    static float GroundY(float, float) { return 0.0f; }  // replace if terrain isn't flat

    Matrix BodyMatrix() const {
        return MatrixMultiply(MatrixRotateY(yaw), MatrixTranslate(pos.x, pos.y, pos.z));
    }

    static float AngleLerp(float a, float b, float t) {
        float d = fmodf(b - a + PI, 2 * PI);
        if (d < 0) d += 2 * PI;
        return a + (d - PI) * t;
    }

    void Update(float dt, Vector3 target) {
        // 1. Steering: smooth velocity toward the target
        Vector3 to = Vector3Subtract(target, pos); to.y = 0;
        float dist = Vector3Length(to);
        Vector3 desired = dist > 0.5f ? Vector3Scale(to, speed / dist) : Vector3Zero();
        vel = Vector3Lerp(vel, desired, 1.0f - expf(-accel * dt));
        pos.x += vel.x * dt;
        pos.z += vel.z * dt;

        if (Vector3Length(vel) > 0.2f)
            yaw = AngleLerp(yaw, atan2f(vel.x, vel.z), 1.0f - expf(-8.0f * dt));

        // 2. Foot stepping
        Matrix bm = BodyMatrix();
        float avgFootY = 0;
        for (int i = 0; i < 4; i++) {
            Leg& l = legs[i];
            avgFootY += l.foot.y * 0.25f;

            if (l.stepT >= 1.0f) {
                Vector3 ideal = Vector3Transform(l.restLocal, bm);
                ideal = Vector3Add(ideal, Vector3Scale(vel, stepLead));
                ideal.y = GroundY(ideal.x, ideal.z);

                float d = Vector2Distance({ l.foot.x, l.foot.z }, { ideal.x, ideal.z });
                bool otherGroupStepping = false;
                for (auto& o : legs)
                    if (o.group != l.group && o.stepT < 1.0f) otherGroupStepping = true;

                if (d > stepDist && !otherGroupStepping) {
                    l.stepFrom = l.foot; l.stepTo = ideal; l.stepT = 0.0f;
                }
            }
            else {
                l.stepT = fminf(1.0f, l.stepT + dt * stepSpeed * (0.5f + Vector3Length(vel) / speed));
                l.foot = Vector3Lerp(l.stepFrom, l.stepTo, l.stepT);
                l.foot.y += sinf(l.stepT * PI) * stepHeight;
            }
        }

        pos.y = Lerp(pos.y, avgFootY + rideHeight, 1.0f - expf(-10.0f * dt));
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

    void Draw() {
        if (!IsModelValid(body) || !IsModelValid(upper) || !IsModelValid(lower)) return;
        Matrix bm = BodyMatrix();
        body.transform = MatrixMultiply(MatrixScale(scale, scale, scale), bm);
        DrawModel(body, Vector3Zero(), 1.0f, GRAY);
        for (auto& l : legs) {
            Vector3 hip = Vector3Transform(l.hipLocal, bm);
            Vector3 foot = l.foot;
            Vector3 knee = SolveKnee(hip, foot, { 0, 1, 0 });  // knees bend upward
            DrawSegment(upper, hip, knee);
            DrawSegment(lower, knee, foot);
        }
    }
};