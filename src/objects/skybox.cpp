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

#include "../core/renderer.hpp"
#include "./skybox.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

/* Public API */

R3D_Skybox R3D_LoadSkybox(const char* fileName, CubemapLayout layout)
{
    void *sky = new r3d::Skybox(fileName, layout);

    return {
        .rotation = (Vector3) { 0, 0, 0 },
        .internal = sky
    };
}

R3D_Skybox R3D_LoadSkyboxHDR(const char* fileName, int sizeFace)
{
    void *sky = new r3d::Skybox(fileName, sizeFace);

    return {
        .rotation = (Vector3) { 0, 0, 0 },
        .internal = sky
    };
}

void R3D_UnloadSkybox(R3D_Skybox* skybox)
{
    if (gRenderer && gRenderer->environment.world.skybox == skybox) {
        gRenderer->environment.world.skybox = nullptr;
    }

    delete static_cast<r3d::Skybox*>(skybox->internal);
}
