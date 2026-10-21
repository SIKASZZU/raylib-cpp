

#include <raylib.h>
#include <raymath.h>

#include "model_assets.hpp"
#include "robot.hpp"


void Robot::Init(const ModelAssets& models) {
    // init body parts
    body = models.at(ModelId::robotBody);
    upper = models.at(ModelId::robotUpper);
    lower = models.at(ModelId::robotLower);

    l1 *= scale;
    l2 *= scale;
    speed *= scale;
    stepDist *= scale;
    stepHeight *= scale;
    rideHeight *= scale;
    arriveDist *= scale;
    pos.y = rideHeight;

    // hip offsets
    float x = 0.5f * scale;
    float z = 0.6f * scale;

    // FL, FR, BL, BR. Diagonals share a group.
    legs[0] = {
        { -x, 0, z },
        { -x * 1.8f, 0, z * 1.5f },  // where the foot wants to stand, which is further out (1.8× sideways) and further ahead or behind (1.5×) than the hip, so the legs splay.
        {},{},{},
        1.0f,
        0
    };
    legs[1] = { { x, 0, z },{ x * 1.8f, 0,  z * 1.5f },{},{},{}, 1.0f, 1 };
    legs[2] = { { -x, 0, -z },{ -x * 1.8f, 0, -z * 1.5f },{},{},{}, 1.0f, 1 };
    legs[3] = { { x, 0, -z },{ x * 1.8f, 0, -z * 1.5f },{},{},{}, 1.0f, 0 };

    Matrix m = BodyMatrix();
    for (auto& l : legs) {
        l.foot = Vector3Transform(l.restLocal, m);
        l.foot.y = GroundY(l.foot.x, l.foot.z);
    }
}


void Robot::Update(float dT, Vector3 targetPosition) {
    // 1. Steering: smooth velocity toward the target
    // vahevektor
    Vector3 to = Vector3Subtract(targetPosition, pos);
    to.y = 0;
    float dist = Vector3Length(to);
    Vector3 desired = dist > arriveDist ? Vector3Scale(to, speed / dist) : Vector3Zero();
    // frame-rate-independent smoothing
    vel = Vector3Lerp(vel, desired, 1.0f - expf(-accel * dT));
    pos.x += vel.x * dT;
    pos.z += vel.z * dT;

    // turning
    if (Vector3Length(vel) > (0.2f * scale)) {
        float turnRate{ -16.0f };
        // The speed check stops the heading jittering when the robot is nearly still. 
        // atan2(x, z) is the angle from +Z toward +X, which matches MatrixRotateY, so “forward” is +Z. 
        yaw = AngleLerp(yaw, atan2f(vel.x, vel.z), 1.0f - expf(turnRate * dT));
    }

    // 2. Foot stepping
    Matrix bm = BodyMatrix();
    float avgFootY = 0;
    for (Leg& l : legs) {
        avgFootY += l.foot.y * (1.0f / std::size(legs));

        // A planted foot (stepT >= 1) does nothing except check whether it should step.
        if (l.stepT >= 1.0f) {
            Vector3 ideal = Vector3Transform(l.restLocal, bm);
            ideal = Vector3Add(ideal, Vector3Scale(vel, stepLead));
            ideal.y = GroundY(ideal.x, ideal.z);

            // where this foot would ideally stand right now
            float d = Vector2Distance({ l.foot.x, l.foot.z }, { ideal.x, ideal.z });
            // the horizontal distance between where the foot is and where it wants to be
            bool otherGroupStepping = false;
            for (auto& o : legs)
                if (o.group != l.group && o.stepT < 1.0f) {
                    otherGroupStepping = true;
                }

            if (d > stepDist && !otherGroupStepping) {
                l.stepFrom = l.foot;
                l.stepTo = ideal;
                l.stepT = 0.0f;
            }
        }
        else {
            // airborne foot
            l.stepT = fminf(1.0f, l.stepT + dT * stepSpeed * (0.5f + Vector3Length(vel) / speed));
            // shift foot from start to target
            l.foot = Vector3Lerp(l.stepFrom, l.stepTo, l.stepT);
            // add an arc
            l.foot.y += sinf(l.stepT * PI) * stepHeight;
        }
    }

    // set body height based on average foot height
    pos.y = Lerp(pos.y, avgFootY + rideHeight, 1.0f - expf(-10.0f * dT));
}


void Robot::Draw() {
    if (!IsModelValid(body) || !IsModelValid(upper) || !IsModelValid(lower)) return;
    Matrix bm = BodyMatrix();
    // MatrixMultiply(a, b) applies a first, so this scales, then rotates and translates.
    body.transform = MatrixMultiply(MatrixScale(scale, scale, scale), bm);

    DrawModel(body, Vector3Zero(), 1.0f, GRAY);

    for (auto& leg : legs) {
        // Moves the hip from body space to world space. This is why the hips follow the body.
        Vector3 hip = Vector3Transform(leg.hipLocal, bm);
        Vector3 foot = leg.foot;
        Vector3 knee = SolveKnee(hip, foot, { 0, 1, 0 });  // knees bend upward
        DrawSegment(upper, hip, knee);
        DrawSegment(lower, knee, foot);
    }
}




