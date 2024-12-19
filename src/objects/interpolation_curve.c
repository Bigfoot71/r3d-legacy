/*
 * Copyright (c) 2024 Le Juez Victor
 * 
 * This software is provided "as-is", without any express or implied warranty. In no event 
 * will the authors be held liable for any damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose, including commercial 
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 * 
 *   1. The origin of this software must not be misrepresented; you must not claim that you 
 *   wrote the original software. If you use this software in a product, an acknowledgment 
 *   in the product documentation would be appreciated but is not required.
 * 
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 * 
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "r3d.h"

#include "../detail/Math.h"

#include <raymath.h>
#include <stdlib.h>

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
        // We resize the buffer to the next power of 2
        unsigned int newCapacity = nextPOT32(curve->capacity);
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
