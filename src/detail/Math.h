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

#ifndef R3D_DETAIL_MATH_H
#define R3D_DETAIL_MATH_H

#include "r3d.h"

#include <raylib.h>
#include <raymath.h>

#ifdef __cplusplus

namespace r3d {

/**
 * @brief Extracts the translation part of a matrix.
 * 
 * This function retrieves the translation components (m12, m13, m14)
 * from the given transformation matrix and returns them as a 3D vector.
 * 
 * @param mat The transformation matrix from which to extract the translation part.
 * @return Vector3 The extracted translation vector (x, y, z).
 */
inline Vector3 getMatrixTrasnlation(const Matrix& mat)
{
    return {
        mat.m12,
        mat.m13,
        mat.m14
    };
}

/**
 * @brief Extracts the scale components of a transformation matrix.
 * 
 * This function retrieves the scale factors along the X, Y, and Z axes
 * from the given transformation matrix. It uses an approximation by
 * summing the absolute values of the components of each axis vector.
 * 
 * Note: The correct formula for precise scale extraction would involve
 * computing the Euclidean norm (length) of the axis vectors instead
 * of summing their absolute values. This implementation provides a
 * faster approximation.
 * 
 * @param mat The transformation matrix from which to extract the scale components.
 * @return Vector3 The extracted scale vector (x, y, z).
 */
inline Vector3 getMatrixScale(const Matrix& mat)
{
    Vector3 scale;

    // Quick approximation of the norm: sum of absolute values of the components.
    scale.x = fabsf(mat.m0) + fabsf(mat.m1) + fabsf(mat.m2);
    scale.y = fabsf(mat.m4) + fabsf(mat.m5) + fabsf(mat.m6);
    scale.z = fabsf(mat.m8) + fabsf(mat.m9) + fabsf(mat.m10);

    // The correct formula would be the following:
    // scale.x = sqrtf(mat.m0 * mat.m0 + mat.m1 * mat.m1 + mat.m2 * mat.m2);
    // scale.y = sqrtf(mat.m4 * mat.m4 + mat.m5 * mat.m5 + mat.m6 * mat.m6);
    // scale.z = sqrtf(mat.m8 * mat.m8 + mat.m9 * mat.m9 + mat.m10 * mat.m10);

    return scale;
}

/**
 * @brief Determines which cube map face corresponds to the given direction.
 * 
 * This function checks the absolute values of the direction vector's components (X, Y, Z) 
 * and determines which face of a cube map the direction vector points to. It is used for 
 * mapping a direction to one of the six cube map faces.
 * 
 * The direction does not need to be normalized for this function to work correctly.
 * 
 * @param direction The direction vector (a `Vector3`) to check.
 * 
 * @return An integer representing the cube map face:
 * - 0: +X face
 * - 1: -X face
 * - 2: +Y face
 * - 3: -Y face
 * - 4: +Z face
 * - 5: -Z face
 */
inline int getCubeMapFace(const Vector3& direction)
{
    float absX = std::abs(direction.x);
    float absY = std::abs(direction.y);
    float absZ = std::abs(direction.z);

    if (absX >= absY && absX >= absZ) {
        return (direction.x > 0) ? 0 : 1;  // +X or -X
    } else if (absY >= absX && absY >= absZ) {
        return (direction.y > 0) ? 2 : 3;  // +Y or -Y
    } else {
        return (direction.z > 0) ? 4 : 5;  // +Z or -Z
    }
}

/**
 * @brief Computes a billboard rotation matrix to align a 3D object with the camera.
 *
 * This function generates a rotation matrix that aligns a 3D object (the "billboard") 
 * towards the camera based on the specified billboard mode. Two modes are supported:
 * - `R3D_BILLBOARD_ENABLED`: The object fully faces the camera, rotating on all axes.
 * - `R3D_BILLBOARD_Y_AXIS`: The object is constrained to rotate only around the Y-axis, 
 *   which is commonly used for objects that should remain upright (e.g., characters or signs).
 *
 * @param mode The billboard mode to use (e.g., `R3D_BILLBOARD_ENABLED` or `R3D_BILLBOARD_Y_AXIS`).
 * @param modelPos The position of the 3D object (billboard) in the world.
 * @param viewPos The position of the camera in the world.
 * @return A rotation matrix that aligns the object to face the camera based on the specified mode.
 */
inline Matrix getBillboardRotationMatrix(R3D_BillboardMode mode, const Vector3 modelPos, const Vector3& viewPos)
{
    Matrix billboardRotation = MatrixIdentity();

    switch (mode) {
        case R3D_BILLBOARD_ENABLED: {
            // Full billboard: faces the camera on all axes

            // Compute direction from the model to the camera
            Vector3 toCamera = Vector3Subtract(viewPos, modelPos);
            toCamera = Vector3Normalize(toCamera);

            // Create a "look-at" alignment using the camera direction
            Vector3 up = { 0.0f, 1.0f, 0.0f }; // Global up axis
            Vector3 right = Vector3Normalize(Vector3CrossProduct(up, toCamera)); // X-axis of the billboard
            up = Vector3CrossProduct(toCamera, right); // Recalculate the Y-axis

            // Build the rotation matrix using the aligned axes
            billboardRotation = {
                right.x, up.x, toCamera.x, 0.0f,
                right.y, up.y, toCamera.y, 0.0f,
                right.z, up.z, toCamera.z, 0.0f,
                0.0f,    0.0f,     0.0f,   1.0f
            };
        } break;

        case R3D_BILLBOARD_Y_AXIS: {
            // Y-axis constrained billboard: rotates only around the Y-axis

            // Compute direction from the model to the camera, projected on the XZ plane
            Vector3 toCamera = Vector3Subtract(viewPos, modelPos);
            toCamera.y = 0.0f; // Ignore the Y component
            toCamera = Vector3Normalize(toCamera);

            // Compute the X-axis
            Vector3 up = { 0.0f, 1.0f, 0.0f }; // Global up axis (Y-axis)
            Vector3 right = Vector3CrossProduct(up, toCamera);

            // Build the rotation matrix
            billboardRotation = {
                right.x, up.x, toCamera.x, 0.0f,
                right.y, up.y, toCamera.y, 0.0f,
                right.z, up.z, toCamera.z, 0.0f,
                0.0f,    0.0f,     0.0f,   1.0f
            };
        } break;

        default:
            // If no valid mode is provided, return the identity matrix
            break;
    }

    return billboardRotation;
}

} // namespace r3d

#else // C

#include <stdint.h>

static inline uint32_t nextPOT32(uint32_t value)
{
    if (value == 0) {
        return 1;
    }

    if ((value & (value - 1)) == 0) {
        return value << 1;
    }

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;

    return value;
}

static inline uint64_t nextPOT64(uint64_t value)
{
    if (value == 0) {
        return 1;
    }

    if ((value & (value - 1)) == 0) {
        return value << 1;
    }

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;

    return value;
}

#endif // __cplusplus
#endif // R3D_DETAIL_MATH_HPP
