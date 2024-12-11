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

#include <raylib.h>
#include <raymath.h>

#include <memory>
#include <cstdio>
#include <cfloat>


// NOTE: Declared in 'src/renderer.hpp'
std::unique_ptr<r3d::Renderer> gRenderer{};


/* API functions */

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
    if (blitAspectKeep) gRenderer->flags |= R3D_FLAG_ASPECT_KEEP;
    else gRenderer->flags &= ~R3D_FLAG_ASPECT_KEEP;

    if (blitLinear) gRenderer->flags |= R3D_FLAG_BLIT_LINEAR;
    else gRenderer->flags &= ~R3D_FLAG_BLIT_LINEAR;
}

void R3D_SetFrustumCulling(bool enabled)
{
    if (enabled) gRenderer->flags &= ~R3D_FLAG_NO_FRUSTUM_CULLING;
    else gRenderer->flags |= R3D_FLAG_NO_FRUSTUM_CULLING;
}

void R3D_SetRenderTarget(const RenderTexture* target)
{
    gRenderer->customRenderTarget = target;
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
