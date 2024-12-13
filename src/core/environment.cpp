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

#include "./renderer.hpp"

/* Public API */

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

R3D_Skybox* R3D_GetEnvWorldSkybox()
{
    return gRenderer->environment.world.skybox;
}

void R3D_SetEnvWorldSkybox(R3D_Skybox* skybox)
{
    gRenderer->environment.world.skybox = skybox;
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
