#include "MahonyAHRS.h"

#include "common.h"
#include "math.h"
#include "string.h"

void MahonyAHRS_init(MahonyAHRS *obj, float _twoKp, float _twoKi,
                     float _sampleFreq) {
    obj->twoKp = _twoKp;
    obj->twoKi = _twoKi;
    obj->sampleFreq = _sampleFreq;
    MahonyAHRS_clear(obj);
}
void MahonyAHRS_clear(MahonyAHRS *obj) {
    obj->q0 = 1.0f;
    obj->q1 = obj->q2 = obj->q3 = 0.0f;
    obj->integralFBx = obj->integralFBy = obj->integralFBz = 0.0f;
    memset(obj->euler, 0, sizeof(float) * 3);
}
void MahonyAHRS_update(MahonyAHRS *obj, float gx, float gy, float gz, float ax,
                       float ay, float az) {
    float recipNorm;
    float halfvx, halfvy, halfvz;
    float halfex, halfey, halfez;
    float qa, qb, qc;

    // Compute feedback only if accelerometer measurement valid (avoids NaN in
    // accelerometer normalisation)
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Estimated direction of gravity and vector perpendicular to magnetic
        // flux
        halfvx = obj->q1 * obj->q3 - obj->q0 * obj->q2;
        halfvy = obj->q0 * obj->q1 + obj->q2 * obj->q3;
        halfvz = obj->q0 * obj->q0 - 0.5f + obj->q3 * obj->q3;

        // Error is sum of cross product between estimated and measured
        // direction of gravity
        halfex = (ay * halfvz - az * halfvy);
        halfey = (az * halfvx - ax * halfvz);
        halfez = (ax * halfvy - ay * halfvx);

        // Compute and apply integral feedback if enabled
        if (obj->twoKi > 0.0f) {
            obj->integralFBx +=
                obj->twoKi * halfex *
                (1.0f / obj->sampleFreq);  // integral error scaled by Ki
            obj->integralFBy += obj->twoKi * halfey * (1.0f / obj->sampleFreq);
            obj->integralFBz += obj->twoKi * halfez * (1.0f / obj->sampleFreq);
            gx += obj->integralFBx;  // apply integral feedback
            gy += obj->integralFBy;
            gz += obj->integralFBz;
        } else {
            obj->integralFBx = 0.0f;  // prevent integral windup
            obj->integralFBy = 0.0f;
            obj->integralFBz = 0.0f;
        }

        // Apply proportional feedback
        gx += obj->twoKp * halfex;
        gy += obj->twoKp * halfey;
        gz += obj->twoKp * halfez;
    }

    // Integrate rate of change of quaternion
    gx *= (0.5f * (1.0f / obj->sampleFreq));  // pre-multiply common factors
    gy *= (0.5f * (1.0f / obj->sampleFreq));
    gz *= (0.5f * (1.0f / obj->sampleFreq));
    qa = obj->q0;
    qb = obj->q1;
    qc = obj->q2;
    obj->q0 += (-qb * gx - qc * gy - obj->q3 * gz);
    obj->q1 += (qa * gx + qc * gz - obj->q3 * gy);
    obj->q2 += (qa * gy - qb * gz + obj->q3 * gx);
    obj->q3 += (qa * gz + qb * gy - qc * gx);

    // Normalise quaternion
    recipNorm = invSqrt(obj->q0 * obj->q0 + obj->q1 * obj->q1 +
                        obj->q2 * obj->q2 + obj->q3 * obj->q3);
    obj->q0 *= recipNorm;
    obj->q1 *= recipNorm;
    obj->q2 *= recipNorm;
    obj->q3 *= recipNorm;

    // Euler
    obj->euler[2] = atan2(2 * (obj->q0 * obj->q1 + obj->q2 * obj->q3),
                          1 - 2 * (obj->q1 * obj->q1 + obj->q2 * obj->q2));
    obj->euler[1] = asin(2 * (obj->q0 * obj->q2 - obj->q3 * obj->q1));
    obj->euler[0] = atan2(2 * (obj->q0 * obj->q3 + obj->q1 * obj->q2),
                          1 - 2 * (obj->q2 * obj->q2 + obj->q3 * obj->q3));
}
