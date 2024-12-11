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

#include <raymath.h>

/* Public API */

R3D_Transform R3D_CreateTransformIdentity(const R3D_Transform* parent)
{
    return (R3D_Transform) {
        .position = { 0, 0, 0 },
        .rotation = { 0, 0, 0, 1 },
        .scale = { 1, 1, 1 },
        .parent = parent
    };
}

R3D_Transform R3D_TransformFromMatrix(Matrix mat)
{
    R3D_Transform transform;

    // Extract the position (elements from the last column)
    transform.position = (Vector3){ mat.m12, mat.m13, mat.m14 };

    // Extract the scale by measuring the length of the column vectors
    Vector3 scale;
    scale.x = Vector3Length((Vector3){ mat.m0, mat.m1, mat.m2 });
    scale.y = Vector3Length((Vector3){ mat.m4, mat.m5, mat.m6 });
    scale.z = Vector3Length((Vector3){ mat.m8, mat.m9, mat.m10 });
    transform.scale = scale;

    // Remove the scale from the matrix to extract the rotation
    Matrix rotationMatrix = mat;

    // Normalize the column vectors to remove the scale
    if (scale.x != 0) {
        rotationMatrix.m0 /= scale.x;
        rotationMatrix.m1 /= scale.x;
        rotationMatrix.m2 /= scale.x;
    }
    if (scale.y != 0) {
        rotationMatrix.m4 /= scale.y;
        rotationMatrix.m5 /= scale.y;
        rotationMatrix.m6 /= scale.y;
    }
    if (scale.z != 0) {
        rotationMatrix.m8 /= scale.z;
        rotationMatrix.m9 /= scale.z;
        rotationMatrix.m10 /= scale.z;
    }

    // Convert the rotation matrix to a quaternion
    transform.rotation = QuaternionFromMatrix(rotationMatrix);

    // Initialize the parent to NULL
    transform.parent = 0;

    return transform;
}

Matrix R3D_TransformToLocal(const R3D_Transform* transform)
{
    Matrix mat_translation = MatrixTranslate(transform->position.x, transform->position.y, transform->position.z);
    Matrix mat_rotation = QuaternionToMatrix(transform->rotation);
    Matrix mat_scale = MatrixScale(transform->scale.x, transform->scale.y, transform->scale.z);

    return MatrixMultiply(MatrixMultiply(mat_translation, mat_rotation), mat_scale);
}

Matrix R3D_TransformToGlobal(const R3D_Transform* transform)
{
    if (transform->parent) {
        return MatrixMultiply(
            R3D_TransformToGlobal(transform),
            R3D_TransformToLocal(transform)
        );
    }

    return R3D_TransformToLocal(transform);
}
