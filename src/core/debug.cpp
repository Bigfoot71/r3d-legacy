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
