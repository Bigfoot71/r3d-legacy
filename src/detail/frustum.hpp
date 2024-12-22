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

#ifndef R3D_DETAIL_FRUSTUM_HPP
#define R3D_DETAIL_FRUSTUM_HPP

#include <raylib.h>
#include <raymath.h>

#include <array>
#include <cmath>

namespace r3d {

/**
 * @class Frustum
 * @brief Represents a frustum used for frustum culling and visibility tests.
 * 
 * This class is designed to represent a frustum (a pyramidal volume defined by the camera's view and projection matrix) 
 * and provides methods to test whether points, spheres, and axis-aligned bounding boxes (AABBs) are inside the frustum.
 * The frustum is typically used in 3D graphics to determine if objects are within the camera's view and thus should be rendered.
 */
class Frustum
{
public:
    /**
     * @brief Default constructor.
     */
    Frustum() = default;

    /**
     * @brief Constructs a frustum from a view-projection matrix.
     * 
     * @param viewProj The combined view and projection matrix.
     */
    Frustum(const ::Matrix& viewProj);

    /**
     * @brief Constructs a frustum from a separate view and projection matrix.
     * 
     * @param view The view matrix.
     * @param proj The projection matrix.
     */
    Frustum(const ::Matrix& view, const ::Matrix& proj);

    /**
     * @brief Checks if a point is inside the frustum.
     * 
     * @param position The position of the point (as a `Vector3`).
     * @return `true` if the point is inside the frustum, otherwise `false`.
     */
    bool pointIn(const ::Vector3& position) const;

    /**
     * @brief Checks if a point is inside the frustum.
     * 
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     * @param z The z-coordinate of the point.
     * @return `true` if the point is inside the frustum, otherwise `false`.
     */
    bool pointIn(float x, float y, float z) const;

    /**
     * @brief Checks if a sphere is inside the frustum.
     * 
     * @param position The center of the sphere (as a `Vector3`).
     * @param radius The radius of the sphere.
     * @return `true` if the sphere is inside the frustum, otherwise `false`.
     */
    bool sphereIn(const ::Vector3& position, float radius) const;

    /**
     * @brief Checks if an axis-aligned bounding box (AABB) is inside the frustum.
     * 
     * @param aabb The AABB to check.
     * @return `true` if the AABB is inside the frustum, otherwise `false`.
     */
    bool aabbIn(const ::BoundingBox& aabb) const;

private:
    /**
     * @brief Enum representing the six planes of the frustum.
     */
    enum Plane : int
    {
        BACK = 0,   ///< The back plane of the frustum.
        FRONT,      ///< The front plane of the frustum.
        BOTTOM,     ///< The bottom plane of the frustum.
        TOP,        ///< The top plane of the frustum.
        RIGHT,      ///< The right plane of the frustum.
        LEFT,       ///< The left plane of the frustum.
    };

private:
    std::array<Vector4, 6> mPlanes; ///< Array holding the six planes of the frustum.

private:
    /**
     * @brief Normalizes a plane equation.
     * 
     * @param plane The plane to normalize.
     * @return The normalized plane.
     */
    static Vector4 normalizePlane(const ::Vector4& plane) {
        float mag = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
        if (mag > 1e-6f) return Vector4Scale(plane, 1.0f / mag);
        return {};  // Return a default zero plane if magnitude is too small.
    }

    /**
     * @brief Calculates the distance from a point to a plane.
     * 
     * @param plane The plane.
     * @param position The position of the point (as a `Vector3`).
     * @return The distance from the point to the plane.
     */
    static float distanceToPlane(const ::Vector4& plane, const ::Vector3& position) {
        return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
    }

    /**
     * @brief Calculates the distance from a point to a plane.
     * 
     * @param plane The plane.
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     * @param z The z-coordinate of the point.
     * @return The distance from the point to the plane.
     */
    static float distanceToPlane(const ::Vector4& plane, float x, float y, float z) {
        return plane.x * x + plane.y * y + plane.z * z + plane.w;
    }
};


/* Implementation */

inline Frustum::Frustum(const ::Matrix& viewProj)
{
    mPlanes[RIGHT] = normalizePlane({
        viewProj.m3 - viewProj.m0,
        viewProj.m7 - viewProj.m4,
        viewProj.m11 - viewProj.m8,
        viewProj.m15 - viewProj.m12
    });

    mPlanes[LEFT] = normalizePlane({
        viewProj.m3 + viewProj.m0,
        viewProj.m7 + viewProj.m4,
        viewProj.m11 + viewProj.m8,
        viewProj.m15 + viewProj.m12
    });

    mPlanes[TOP] = normalizePlane({
        viewProj.m3 - viewProj.m1,
        viewProj.m7 - viewProj.m5,
        viewProj.m11 - viewProj.m9,
        viewProj.m15 - viewProj.m13
    });

    mPlanes[BOTTOM] = normalizePlane({
        viewProj.m3 + viewProj.m1,
        viewProj.m7 + viewProj.m5,
        viewProj.m11 + viewProj.m9,
        viewProj.m15 + viewProj.m13
    });

    mPlanes[BACK] = normalizePlane({
        viewProj.m3 - viewProj.m2,
        viewProj.m7 - viewProj.m6,
        viewProj.m11 - viewProj.m10,
        viewProj.m15 - viewProj.m14
    });

    mPlanes[FRONT] = normalizePlane({
        viewProj.m3 + viewProj.m2,
        viewProj.m7 + viewProj.m6,
        viewProj.m11 + viewProj.m10,
        viewProj.m15 + viewProj.m14
    });
}

inline Frustum::Frustum(const ::Matrix& view, const ::Matrix& proj)
    : Frustum(MatrixMultiply(view, proj))
{ }

inline bool Frustum::pointIn(const ::Vector3& position) const
{
    for (auto& plane : mPlanes) {
        if (distanceToPlane(plane, position) <= 0) { // point is behind plane
            return false;
        }
    }
    return true;
}

inline bool Frustum::pointIn(float x, float y, float z) const
{
    for (auto& plane : mPlanes) {
        if (distanceToPlane(plane, x, y, z) <= 0) { // point is behind plane
            return false;
        }
    }
    return true;
}

inline bool Frustum::sphereIn(const ::Vector3& position, float radius) const
{
    for (auto& plane : mPlanes) {
        if (distanceToPlane(plane, position) < -radius) { // center is behind plane by more than the radius
            return false;
        }
    }
    return true;
}

inline bool Frustum::aabbIn(const ::BoundingBox& aabb) const
{
    // if any point is in and we are good
    if (pointIn(aabb.min.x, aabb.min.y, aabb.min.z)) return true;
    if (pointIn(aabb.min.x, aabb.max.y, aabb.min.z)) return true;
    if (pointIn(aabb.max.x, aabb.max.y, aabb.min.z)) return true;
    if (pointIn(aabb.max.x, aabb.min.y, aabb.min.z)) return true;
    if (pointIn(aabb.min.x, aabb.min.y, aabb.max.z)) return true;
    if (pointIn(aabb.min.x, aabb.max.y, aabb.max.z)) return true;
    if (pointIn(aabb.max.x, aabb.max.y, aabb.max.z)) return true;
    if (pointIn(aabb.max.x, aabb.min.y, aabb.max.z)) return true;

    // check to see if all points are outside of any one plane, if so the entire box is outside
    for (auto& plane : mPlanes) {
        bool oneInside = false;
        if (distanceToPlane(plane, aabb.min.x, aabb.min.y, aabb.min.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.max.x, aabb.min.y, aabb.min.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.max.x, aabb.max.y, aabb.min.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.min.x, aabb.max.y, aabb.min.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.min.x, aabb.min.y, aabb.max.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.max.x, aabb.min.y, aabb.max.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.max.x, aabb.max.y, aabb.max.z) >= 0) oneInside = true;
        if (distanceToPlane(plane, aabb.min.x, aabb.max.y, aabb.max.z) >= 0) oneInside = true;
        if (!oneInside) return false;
    }

    // the box extends outside the frustum but crosses it
    return true;
}

} // namespace r3d

#endif // R3D_DETAIL_FRUSTUM_HPP
