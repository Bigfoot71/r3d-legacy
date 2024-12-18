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

#ifndef R3D_LIGHTING_HPP
#define R3D_LIGHTING_HPP

#include "r3d.h"

#include "../detail/GL/GLFramebuffer.hpp"
#include "../detail/RenderTarget.hpp"
#include "../detail/Frustum.hpp"
#include "../detail/Math.h"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <optional>

namespace r3d {

/**
 * @brief Generic container used by the Renderer to represent any type of lighting.
 */
struct Light
{
    std::optional<RenderTarget> map; ///< Optional shadow map for the light.
    Frustum frustum;                 ///< Frustum from the point of view of light.

    Color color;                     ///< The color of the light.
    Vector3 position;                ///< The position of the light in world space.
    Vector3 direction;               ///< The direction the light is pointing towards.
    float energy;                    ///< The intensity of the light.
    float maxDistance;               ///< Maximum range of the light's effect.
    float attenuation;               ///< Attenuation factor of the light's intensity over distance.
    float innerCutOff;               ///< Inner cone angle for spotlights (in degrees).
    float outerCutOff;               ///< Outer cone angle for spotlights (in degrees).
    float shadowBias;                ///< Bias to reduce shadow artifacts.
    bool shadow;                     ///< Flag indicating whether the light casts shadows.
    bool enabled;                    ///< Flag indicating whether the light is active.
    R3D_LightType type;              ///< Type of the light (e.g., directional, point, spotlight).
    int layers;                      ///< Represents the layers (`R3D_Layer`) in which the light illuminates

    /**
     * @brief Constructs a light of the specified type.
     * 
     * @param type The type of the light.
     * @param shadowMapResolution Resolution of the shadow map. Specify '0' to disable shadows.
     */
    Light(R3D_LightType type, int shadowMapResolution = 2048);

    /**
     * @brief Initializes shadow rendering by allocating a shadow map.
     * 
     * @param shadowMapResolution Resolution of the shadow map.
     */
    void enableShadow(int shadowMapResolution);

    /**
     * @brief Disables shadow rendering and deallocates the shadow map.
     */
    void disableShadow();

    /**
     * @brief Updates the frustum based on the current position and direction of the light.
     * 
     * This method recalculates the frustum used for culling shadows based on the light's 
     * current position and direction. It is typically called whenever the light's position 
     * or direction is changed, ensuring the correct frustum is used for shadow rendering 
     * and visibility tests.
     */
    void updateFrustum();

    /**
     * @brief Returns the view matrix of the light for shadow rendering.
     * 
     * @param face For omnidirectional lights (using cubemaps), specify the cubemap face. Leave empty for other light types.
     * @return The view matrix of the light.
     */
    Matrix viewMatrix(int face = -1) const;

    /**
     * @brief Returns the projection matrix of the light for shadow rendering.
     * 
     * @return The projection matrix of the light.
     */
    Matrix projMatrix() const;

    /**
     * @brief Returns the combined view/projection matrix of the light for shadow rendering.
     * 
     * @param face For omnidirectional lights (using cubemaps), specify the cubemap face. Leave empty for other light types.
     * @return The combined view/projection matrix of the light.
     */
    Matrix vpMatrix(int face = -1) const;
};

/* Public implementation */

inline Light::Light(R3D_LightType type, int shadowMapResolution)
    : color(WHITE)
    , position()
    , direction(0.0f, 0.0f, -1.0f)
    , energy(1.0f)
    , maxDistance(32.0f)
    , attenuation(1.0f)
    , innerCutOff(-1.0f)
    , outerCutOff(-1.0f)
    , shadowBias(0.0f)
    , shadow(shadowMapResolution > 0)
    , enabled(false)
    , type(type)
    , layers(R3D_LAYER_0)
{
    if (shadow) {
        enableShadow(shadowMapResolution);
        if (type != R3D_OMNILIGHT) {
            updateFrustum();
        }
    }
}

inline void Light::enableShadow(int shadowMapResolution)
{
    shadow = true;

    if (map == std::nullopt && shadowMapResolution > 0) {
        map.emplace(shadowMapResolution, shadowMapResolution);
        switch (type) {
            case R3D_DIRLIGHT:
            case R3D_SPOTLIGHT: {
                auto& texture = map->createAttachment(
                    GLAttachement::DEPTH, GL_TEXTURE_2D,
                    GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT,
                    GL_UNSIGNED_SHORT
                );
                texture.wrap(GLTexture::Wrap::CLAMP_BORDER);
                texture.filter(GLTexture::Filter::NEAREST);
                texture.borderColor(WHITE);
                map->setDrawBuffer(GLAttachement::NONE);
                map->setReadBuffer(GLAttachement::NONE);
            } break;
            case R3D_OMNILIGHT: {
                auto& texDepth = map->createAttachment(
                    GLAttachement::DEPTH, GL_TEXTURE_2D,
                    GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT,
                    GL_UNSIGNED_SHORT
                );
                texDepth.wrap(GLTexture::Wrap::CLAMP_EDGE);
                texDepth.filter(GLTexture::Filter::NEAREST);
                auto& texColor = map->createAttachment(
                    GLAttachement::COLOR_0, GL_TEXTURE_CUBE_MAP,
                    GL_R16F, GL_RED, GL_FLOAT
                );
                texColor.wrap(GLTexture::Wrap::CLAMP_EDGE);
                texColor.filter(GLTexture::Filter::NEAREST);
                map->setDrawBuffer(GLAttachement::NONE);
                map->setReadBuffer(GLAttachement::NONE);
            } break;
        }
    }
}

inline void Light::disableShadow()
{
    shadow = false;
    map.reset();
}

inline void Light::updateFrustum()
{
    // Updating the frustum for an omnilight here doesn't make sense;
    // we shouldn't use it for omnilights, but I'll leave it just in case.

    frustum = Frustum(vpMatrix(
        (type == R3D_OMNILIGHT) ? getCubeMapFace(direction) : -1
    ));
}

inline Matrix Light::viewMatrix(int face) const
{
    if (type == R3D_OMNILIGHT) {
        assert(face >= 0 && face <= 6 && "Face out of bounds");
        static constexpr Vector3 dirs[6] = {
            {  1.0,  0.0,  0.0 }, // +X
            { -1.0,  0.0,  0.0 }, // -X
            {  0.0,  1.0,  0.0 }, // +Y
            {  0.0, -1.0,  0.0 }, // -Y
            {  0.0,  0.0,  1.0 }, // +Z
            {  0.0,  0.0, -1.0 }  // -Z
        };
        static constexpr Vector3 ups[6] = {
            {  0.0, -1.0,  0.0 }, // +X
            {  0.0, -1.0,  0.0 }, // -X
            {  0.0,  0.0,  1.0 }, // +Y
            {  0.0,  0.0, -1.0 }, // -Y
            {  0.0, -1.0,  0.0 }, // +Z
            {  0.0, -1.0,  0.0 }  // -Z
        };
        return MatrixLookAt(position, Vector3Add(position, dirs[face]), ups[face]);
    }
    return MatrixLookAt(position, Vector3Add(position, direction), { 0, 1, 0 });
}

inline Matrix Light::projMatrix() const
{
    if (type == R3D_DIRLIGHT) {
        return MatrixOrtho(-10, 10, -10, 10, 0.05, 4000.0);
    }
    return MatrixPerspective(90*DEG2RAD, 1.0, 0.05, maxDistance);
}

inline Matrix Light::vpMatrix(int face) const
{
    return MatrixMultiply(viewMatrix(face), projMatrix());
}

} // namespace r3d

#endif // R3D_LIGHTING_HPP
