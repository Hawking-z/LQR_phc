#include "MadgwickAHRS.h"

#include "common.h"
#include "math.h"
#include "string.h"
void MadgwickAHRS_init(MadgwickAHRS *obj, float _beta, float _sampleFreq) {
    obj->beta = _beta;
    obj->sampleFreq = _sampleFreq;
    MadgwickAHRS_clear(obj);
}
void MadgwickAHRS_clear(MadgwickAHRS *obj) {
    obj->q0 = 1.0f;
    obj->q1 = obj->q2 = obj->q3 = 0.0f;
    memset(obj->euler, 0, sizeof(float) * 3);
}
void MadgwickAHRS_update(MadgwickAHRS *obj, float gx, float gy, float gz,
                         float ax, float ay, float az) {
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2, q0q0, q1q1,
        q2q2, q3q3;

    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5f * (-obj->q1 * gx - obj->q2 * gy - obj->q3 * gz);
    qDot2 = 0.5f * (obj->q0 * gx + obj->q2 * gz - obj->q3 * gy);
    qDot3 = 0.5f * (obj->q0 * gy - obj->q1 * gz + obj->q3 * gx);
    qDot4 = 0.5f * (obj->q0 * gz + obj->q1 * gy - obj->q2 * gx);

    // Compute feedback only if accelerometer measurement valid (avoids NaN in
    // accelerometer normalisation)
    if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        // Normalise accelerometer measurement
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0 = 2.0f * obj->q0;
        _2q1 = 2.0f * obj->q1;
        _2q2 = 2.0f * obj->q2;
        _2q3 = 2.0f * obj->q3;
        _4q0 = 4.0f * obj->q0;
        _4q1 = 4.0f * obj->q1;
        _4q2 = 4.0f * obj->q2;
        _8q1 = 8.0f * obj->q1;
        _8q2 = 8.0f * obj->q2;
        q0q0 = obj->q0 * obj->q0;
        q1q1 = obj->q1 * obj->q1;
        q2q2 = obj->q2 * obj->q2;
        q3q3 = obj->q3 * obj->q3;

        // Gradient decent algorithm corrective step
        s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * obj->q1 - _2q0 * ay -
             _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
        s2 = 4.0f * q0q0 * obj->q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay -
             _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
        s3 = 4.0f * q1q1 * obj->q3 - _2q1 * ax + 4.0f * q2q2 * obj->q3 -
             _2q2 * ay;
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 +
                            s3 * s3);  // normalise step magnitude
        s0 *= recipNorm;
        s1 *= recipNorm;
        s2 *= recipNorm;
        s3 *= recipNorm;

        // Apply feedback step
        qDot1 -= obj->beta * s0;
        qDot2 -= obj->beta * s1;
        qDot3 -= obj->beta * s2;
        qDot4 -= obj->beta * s3;
    }

    // Integrate rate of change of quaternion to yield quaternion
    obj->q0 += qDot1 * (1.0f / obj->sampleFreq);
    obj->q1 += qDot2 * (1.0f / obj->sampleFreq);
    obj->q2 += qDot3 * (1.0f / obj->sampleFreq);
    obj->q3 += qDot4 * (1.0f / obj->sampleFreq);

    // Normalise quaternion
    recipNorm = invSqrt(obj->q0 * obj->q0 + obj->q1 * obj->q1 +
                        obj->q2 * obj->q2 + obj->q3 * obj->q3);
    obj->q0 *= recipNorm;
    obj->q1 *= recipNorm;
    obj->q2 *= recipNorm;
    obj->q3 *= recipNorm;

    // Euler
    obj->euler[0] = atan2(2 * (obj->q0 * obj->q1 + obj->q2 * obj->q3),
                          1 - 2 * (obj->q1 * obj->q1 + obj->q2 * obj->q2));
    obj->euler[1] = asin(2 * (obj->q0 * obj->q2 - obj->q3 * obj->q1));
    obj->euler[2] = atan2(2 * (obj->q0 * obj->q3 + obj->q1 * obj->q2),
                          1 - 2 * (obj->q2 * obj->q2 + obj->q3 * obj->q3));
}
