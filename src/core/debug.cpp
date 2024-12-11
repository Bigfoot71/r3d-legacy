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
