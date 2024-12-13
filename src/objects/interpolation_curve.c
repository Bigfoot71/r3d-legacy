#include "r3d.h"

#include <raymath.h>

R3D_InterpolationCurve R3D_LoadInterpolationCurve(int capacity)
{
    R3D_InterpolationCurve curve;

    curve.keyframes = malloc(capacity * sizeof(R3D_Keyframe));
    curve.capacity = capacity;
    curve.size = 0;

    return curve;
}

void R3D_UnloadInterpolationCurve(R3D_InterpolationCurve* curve)
{
    free(curve->keyframes);
    curve->capacity = 0;
    curve->size = 0;
}

bool R3D_AddKeyframe(R3D_InterpolationCurve* curve, float time, float value)
{
    if (curve->size >= curve->capacity) {
        unsigned int newCapacity = curve->capacity * 2;
        R3D_Keyframe *newKeyframes = realloc(curve->keyframes, newCapacity * sizeof(R3D_Keyframe));
        if (!newKeyframes) {
            return false; // Memory allocation failed
        }
        curve->keyframes = newKeyframes;
        curve->capacity = newCapacity;
    }

    curve->keyframes[curve->size++] = (R3D_Keyframe) {
        time, value
    };

    // Sort the keyframes by time
    for (int i = 0; i < curve->size - 1; ++i) {
        for (int j = 0; j < curve->size - i - 1; ++j) {
            if (curve->keyframes[j].time > curve->keyframes[j + 1].time) {
                R3D_Keyframe temp = curve->keyframes[j];
                curve->keyframes[j] = curve->keyframes[j + 1];
                curve->keyframes[j + 1] = temp;
            }
        }
    }

    return true;
}

float R3D_EvaluateCurve(R3D_InterpolationCurve* curve, float time)
{
    if (curve->size == 0) return 0.0f;
    if (time <= curve->keyframes[0].time) return curve->keyframes[0].value;
    if (time >= curve->keyframes[curve->size - 1].time) return curve->keyframes[curve->size - 1].value;

    // Find the two keyframes surrounding the given time
    for (int i = 0; i < curve->size - 1; i++) {
        const R3D_Keyframe *kf1 = &curve->keyframes[i];
        const R3D_Keyframe *kf2 = &curve->keyframes[i + 1];

        if (time >= kf1->time && time <= kf2->time) {
            float t = (time - kf1->time) / (kf2->time - kf1->time); // Normalized time between kf1 and kf2
            return Lerp(kf1->value, kf2->value, t);
        }
    }

    return 0.0f; // Fallback (should not be reached)
}
