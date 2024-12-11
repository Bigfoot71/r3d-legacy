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

#ifndef R3D_PRIV_UTILS_MATH_HPP
#define R3D_PRIV_UTILS_MATH_HPP

#include <raylib.h>
#include <cstdlib>

namespace r3d {

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

} // namespace r3d

#endif
