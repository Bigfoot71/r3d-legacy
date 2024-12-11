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

#ifndef R3D_RL_CAMERA3D_HPP
#define R3D_RL_CAMERA3D_HPP

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace r3d {

struct RLCamera3D : Camera3D
{
    RLCamera3D() = default;

    RLCamera3D(const Camera3D& cam)
        : Camera3D(cam)
    { }

    void begin() const {
        BeginMode3D(*this);
    }

    void end() const {
        EndMode3D();
    }

    Matrix viewMatrix() const {
        return MatrixLookAt(position, target, up);
    }

    Matrix projMatrix(float aspect) const {
        Matrix matProj;
        if (projection == CAMERA_PERSPECTIVE) {
            double top = rlGetCullDistanceNear() * std::tan(fovy * 0.5 * DEG2RAD);
            double right = top * aspect;
            matProj = MatrixFrustum(-right, right, -top, top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
        } else if (projection == CAMERA_ORTHOGRAPHIC) {
            double top = fovy / 2.0;
            double right = top * aspect;
            matProj = MatrixOrtho(-right, right, -top,top, rlGetCullDistanceNear(), rlGetCullDistanceFar());
        }
        return matProj;
    }

    Matrix vpMatrix(float aspect) const {
        return MatrixMultiply(viewMatrix(), projMatrix(aspect));
    }
};

} // namespace r3d

#endif // R3D_RL_CAMERA3D_HPP
