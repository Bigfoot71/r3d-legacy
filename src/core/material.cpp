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
#include "raylib.h"

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
        .uv {
            .offset = Vector2 { 0.0f, 0.0f },
            .scale = Vector2 { 1.0f, 1.0f }
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
