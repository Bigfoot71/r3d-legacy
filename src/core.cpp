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

#include "./detail/Renderer.hpp"
#include "./detail/Skybox.hpp"
#include "./detail/Light.hpp"
#include "./renderer.hpp"

#include <raylib.h>
#include <raymath.h>

#include <memory>
#include <cstdio>
#include <cfloat>

// NOTE: Declared in 'src/renderer.hpp'
std::unique_ptr<r3d::Renderer> gRenderer{};

/* Main functions */

void R3D_Init(void)
{
    gRenderer = std::make_unique<r3d::Renderer>(0, 0, 0);
}

void R3D_InitEx(int internalWidth, int internalHeight, int flags)
{
    gRenderer = std::make_unique<r3d::Renderer>(internalWidth, internalHeight, flags);
}

void R3D_Close()
{
    gRenderer.reset();
}

void R3D_UpdateInternalResolution(int width, int height)
{
    gRenderer->updateInternalResolution(width, height);
}

void R3D_SetBlitMode(bool blitAspectKeep, bool blitLinear)
{
    gRenderer->blitAspectKeep = blitAspectKeep;
    gRenderer->blitLinear = blitLinear;
}

void R3D_Begin(Camera3D camera)
{
    gRenderer->setCamera(camera);
}

void R3D_Draw(const R3D_Model* model)
{
    gRenderer->draw(*model, {}, {}, 0.0f, { 1.0f, 1.0f, 1.0f });
}

void R3D_DrawEx(const R3D_Model* model, Vector3 position, float scale)
{
    gRenderer->draw(*model, position, {}, 0.0f, { scale, scale, scale });
}

void R3D_DrawPro(const R3D_Model* model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale)
{
    gRenderer->draw(*model, position, rotationAxis, rotationAngle, scale);
}

void R3D_End()
{
    gRenderer->present();
}


/* Environment functions */

R3D_Environment* R3D_GetEnvironment(void)
{
    return &gRenderer->environment;
}

void R3D_SetEnvironment(const R3D_Environment* env)
{
    gRenderer->environment = *env;
}

R3D_Bloom R3D_GetEnvBloomMode()
{
    return gRenderer->environment.bloom.mode;
}

float R3D_GetEnvBloomIntensity(void)
{
    return gRenderer->environment.bloom.intensity;
}

void R3D_SetEnvBloomIntensity(float intensity)
{
    gRenderer->environment.bloom.intensity = intensity;
}

float R3D_GetEnvBloomHDRThreshold(void)
{
    return gRenderer->environment.bloom.hdrThreshold;
}

void R3D_SetEnvBloomHDRThreshold(float threshold)
{
    gRenderer->environment.bloom.hdrThreshold = threshold;
}

void R3D_SetEnvBloomMode(R3D_Bloom mode)
{
    gRenderer->environment.bloom.mode = mode;
}

int R3D_GetEnvBloomIterations()
{
    return gRenderer->environment.bloom.iterations;
}

void R3D_SetEnvBloomIterations(int iterations)
{
    gRenderer->environment.bloom.iterations = iterations;
}

R3D_Fog R3D_GetEnvFogMode()
{
    return gRenderer->environment.fog.mode;
}

void R3D_SetEnvFogMode(R3D_Fog mode)
{
    gRenderer->environment.fog.mode = mode;
}

Color R3D_GetEnvFogColor()
{
    return gRenderer->environment.fog.color;
}

void R3D_SetEnvFogColor(Color color)
{
    gRenderer->environment.fog.color = color;
}

float R3D_GetEnvFogStart()
{
    return gRenderer->environment.fog.start;
}

void R3D_SetEnvFogStart(float start)
{
    gRenderer->environment.fog.start = start;
}

float R3D_GetEnvFogEnd()
{
    return gRenderer->environment.fog.end;
}

void R3D_SetEnvFogEnd(float end)
{
    gRenderer->environment.fog.end = end;
}

float R3D_GetEnvFogDensity()
{
    return gRenderer->environment.fog.density;
}

void R3D_SetEnvFogDensity(float density)
{
    gRenderer->environment.fog.density = density;
}

R3D_Tonemap R3D_GetEnvTonemapMode()
{
    return gRenderer->environment.tonemap.mode;
}

void R3D_SetEnvTonemapMode(R3D_Tonemap mode)
{
    gRenderer->environment.tonemap.mode = mode;
}

float R3D_GetEnvTonemapExposure()
{
    return gRenderer->environment.tonemap.exposure;
}

void R3D_SetEnvTonemapExposure(float exposure)
{
    gRenderer->environment.tonemap.exposure = exposure;
}

float R3D_GetEnvTonemapWhite()
{
    return gRenderer->environment.tonemap.white;
}

void R3D_SetEnvTonemapWhite(float white)
{
    gRenderer->environment.tonemap.white = white;
}

float R3D_GetEnvAdjustBrightness()
{
    return gRenderer->environment.adjustements.brightness;
}

void R3D_SetEnvAdjustBrightness(float brightness)
{
    gRenderer->environment.adjustements.brightness = brightness;
}

float R3D_GetEnvAdjustContrast()
{
    return gRenderer->environment.adjustements.contrast;
}

void R3D_SetEnvAdjustContrast(float contrast)
{
    gRenderer->environment.adjustements.contrast = contrast;
}

float R3D_GetEnvAdjustSaturation()
{
    return gRenderer->environment.adjustements.saturation;
}

void R3D_SetEnvAdjustSaturation(float saturation)
{
    gRenderer->environment.adjustements.saturation = saturation;
}

R3D_Skybox R3D_GetEnvWorldSkybox()
{
    return gRenderer->environment.world.skybox;
}

void R3D_SetEnvWorldSkybox(R3D_Skybox skybox)
{
    gRenderer->environment.world.skybox = static_cast<r3d::Skybox*>(skybox);
}

Color R3D_GetEnvWorldAmbient()
{
    return gRenderer->environment.world.ambient;
}

void R3D_SetEnvWorldAmbient(Color color)
{
    gRenderer->environment.world.ambient = color;
}

Color R3D_GetEnvWorldBackground()
{
    return gRenderer->environment.world.background;
}

void R3D_SetEnvWorldBackground(Color color)
{
    gRenderer->environment.world.background = color;
}


/* Material functions */

R3D_MaterialConfig R3D_CreateMaterialConfig(R3D_DiffuseMode diffuse, R3D_SpecularMode specular,
                                            R3D_BlendMode blendMode, R3D_CullMode cullMode,
                                            int flags)
{
    R3D_MaterialConfig config = {
        .shader {
            .diffuse = static_cast<uint8_t>(diffuse),
            .specular = static_cast<uint8_t>(specular),
            .reserved = 0,
            .flags = static_cast<uint8_t>(flags)
        },
        .blendMode = static_cast<uint8_t>(blendMode),
        .cullMode = static_cast<uint8_t>(cullMode),
        .reserved1 = 0,
        .reserved2 = 0
    };

    gRenderer->loadMaterialConfig(config);

    return config;
}

R3D_Material R3D_CreateMaterial(R3D_MaterialConfig config)
{
    return (R3D_Material) {
        .albedo {
            .texture = *R3D_GetDefaultTextureWhite(),
            .color = WHITE
        },
        .metalness {
            .texture = *R3D_GetDefaultTextureWhite(),
            .factor = 0.0f
        },
        .roughness {
            .texture = *R3D_GetDefaultTextureWhite(),
            .factor = 1.0f
        },
        .emission {
            .texture = *R3D_GetDefaultTextureBlack(),
            .color = BLACK
        },
        .normal {
            .texture = *R3D_GetDefaultTextureBlack()
        },
        .ao {
            .texture = *R3D_GetDefaultTextureWhite(),
            .lightAffect = 0.0f
        },
        .config = config
    };
}

R3D_MaterialConfig R3D_GetDefaultMaterialConfig()
{
    return gRenderer->getDefaultMaterialConfig();
}

void R3D_SetDefaultMaterialConfig(R3D_MaterialConfig config)
{
    gRenderer->setDefaultMaterialConfig(config);
}

void R3D_RegisterMaterialConfig(R3D_MaterialConfig config)
{
    gRenderer->loadMaterialConfig(config);
}

void R3D_UnloadMaterialConfig(R3D_MaterialConfig config)
{
    gRenderer->unloadMaterialConfig(config);
}

bool R3D_IsMaterialConfigValid(R3D_MaterialConfig config)
{
    return gRenderer->isMaterialConfigValid(config);
}


/* Lighting functions */

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

    if (l.shadow) {
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

    if (l.shadow) {
        l.updateFrustum();
    }
}

void R3D_SetLightTarget(R3D_Light light, Vector3 target)
{
    auto& l = gRenderer->getLight(light);
    l.direction = Vector3Normalize(Vector3Subtract(target, l.position));

    if (l.shadow) {
        l.updateFrustum();
    }
}

void R3D_SetLightPositionTarget(R3D_Light light, Vector3 position, Vector3 target)
{
    auto& l = gRenderer->getLight(light);
    l.direction = Vector3Normalize(Vector3Subtract(target, position));
    l.position = position;

    if (l.shadow) {
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
    if (shadowMapResolution <= 0) return;

    auto& l = gRenderer->getLight(light);

    if (!l.shadow) {
        l.enableShadow(shadowMapResolution);
        l.updateFrustum();
    }
}

void R3D_DisableLightShadow(R3D_Light light)
{
    auto& l = gRenderer->getLight(light);

    if (!l.shadow) {
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


/* Debug functions */

void R3D_GetDrawCallCount(int* sceneDrawCount, int* shadowDrawCount)
{
    gRenderer->getDrawCallCount(sceneDrawCount, shadowDrawCount);
}

void R3D_DrawShadowMap(R3D_Light light, int x, int y, int width, int height, float zNear, float zFar)
{
    gRenderer->drawShadowMap(light, x, y, width, height, zNear, zFar);
}

const Texture2D* R3D_GetDefaultTextureBlack()
{
    return &gRenderer->getTextureBlack();
}

const Texture2D* R3D_GetDefaultTextureWhite()
{
    return &gRenderer->getTextureWhite();
}
