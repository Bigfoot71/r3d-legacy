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

#include "./lighting.hpp"
#include "./renderer.hpp"

/* Public API */

R3D_Light R3D_CreateLight(R3D_LightType type, int shadowMapResolution)
{
    return gRenderer->addLight(type, shadowMapResolution);
}

void R3D_DestroyLight(R3D_Light light)
{
    gRenderer->removeLight(light);
}

bool R3D_IsLightActive(R3D_Light light)
{
    return gRenderer->getLight(light).enabled;
}

void R3D_SetLightActive(R3D_Light light, bool enabled)
{
    gRenderer->getLight(light).enabled = enabled;
}

void R3D_ToggleLight(R3D_Light light)
{
    auto& l = gRenderer->getLight(light);
    l.enabled = !l.enabled;
}

Color R3D_GetLightColor(R3D_Light light)
{
    return gRenderer->getLight(light).color;
}

void R3D_SetLightColor(R3D_Light light, Color color)
{
    gRenderer->getLight(light).color = color;
}

Vector3 R3D_GetLightPosition(R3D_Light light)
{
    return gRenderer->getLight(light).position;
}

void R3D_SetLightPosition(R3D_Light light, Vector3 position)
{
    auto& l = gRenderer->getLight(light);
    l.position = position;

    if (l.type != R3D_OMNILIGHT) {
        l.updateFrustum();
    }
}

Vector3 R3D_GetLightDirection(R3D_Light light)
{
    return gRenderer->getLight(light).direction;
}

void R3D_SetLightDirection(R3D_Light light, Vector3 direction)
{
    auto& l = gRenderer->getLight(light);
    l.direction = direction;

    if (l.type != R3D_OMNILIGHT) {
        l.updateFrustum();
    }
}

void R3D_SetLightTarget(R3D_Light light, Vector3 target)
{
    auto& l = gRenderer->getLight(light);
    l.direction = Vector3Normalize(Vector3Subtract(target, l.position));

    if (l.type != R3D_OMNILIGHT) {
        l.updateFrustum();
    }
}

void R3D_SetLightPositionTarget(R3D_Light light, Vector3 position, Vector3 target)
{
    auto& l = gRenderer->getLight(light);
    l.direction = Vector3Normalize(Vector3Subtract(target, position));
    l.position = position;

    if (l.type != R3D_OMNILIGHT) {
        l.updateFrustum();
    }
}

float R3D_GetLightEnergy(R3D_Light light)
{
    return gRenderer->getLight(light).energy;
}

void R3D_SetLightEnergy(R3D_Light light, float energy)
{
    gRenderer->getLight(light).energy = energy;
}

float R3D_GetLightRange(R3D_Light light)
{
    return gRenderer->getLight(light).maxDistance;
}

void R3D_SetLightRange(R3D_Light light, float distance)
{
    gRenderer->getLight(light).maxDistance = distance;
}

float R3D_GetLightAttenuation(R3D_Light light)
{
    return gRenderer->getLight(light).attenuation;
}

void R3D_SetLightAttenuation(R3D_Light light, float factor)
{
    gRenderer->getLight(light).attenuation = factor;
}

float R3D_GetLightInnerCutOff(R3D_Light light)
{
    return std::acos(gRenderer->getLight(light).innerCutOff) * RAD2DEG;
}

void R3D_SetLightInnerCutOff(R3D_Light light, float angle)
{
    gRenderer->getLight(light).innerCutOff = std::cos(angle * DEG2RAD);
}

float R3D_GetLightOuterCutOff(R3D_Light light)
{
    return std::acos(gRenderer->getLight(light).outerCutOff) * RAD2DEG;
}

void R3D_SetLightOuterCutOff(R3D_Light light, float angle)
{
    gRenderer->getLight(light).outerCutOff = std::cos(angle * DEG2RAD);
}

float R3D_GetLightShadowBias(R3D_Light light)
{
    return gRenderer->getLight(light).shadowBias;
}

void R3D_SetLightShadowBias(R3D_Light light, float bias)
{
    gRenderer->getLight(light).shadowBias = bias;
}

bool R3D_IsLightProduceShadows(R3D_Light light)
{
    return gRenderer->getLight(light).shadow;
}

void R3D_EnableLightShadow(R3D_Light light, int shadowMapResolution)
{
    auto& l = gRenderer->getLight(light);

    if (!l.shadow && shadowMapResolution > 0) {
        gRenderer->getLight(light).enableShadow(shadowMapResolution);
    }
}

void R3D_DisableLightShadow(R3D_Light light)
{
    auto& l = gRenderer->getLight(light);

    if (l.shadow) {
        l.disableShadow();
    }
}

R3D_LightType R3D_GetLightType(R3D_Light light)
{
    return gRenderer->getLight(light).type;
}

void R3D_SetLightType(R3D_Light light, R3D_LightType type)
{
    gRenderer->getLight(light).type = type;
}

void R3D_SetLightLayers(R3D_Light light, int layers)
{
    gRenderer->getLight(light).layers = layers;
}

int R3D_GetLightLayers(R3D_Light light)
{
    return gRenderer->getLight(light).layers;
}

void R3D_AddLightLayer(R3D_Light light, R3D_Layer layer)
{
    gRenderer->getLight(light).layers |= layer;
}

void R3D_RemoveLightLayer(R3D_Light light, R3D_Layer layer)
{
    gRenderer->getLight(light).layers &= ~layer;
}

void R3D_ToggleLightLayer(R3D_Light light, R3D_Layer layer)
{
    auto& l = gRenderer->getLight(light);
    if (l.layers & layer) l.layers &= ~layer;
    else l.layers |= layer;
}
