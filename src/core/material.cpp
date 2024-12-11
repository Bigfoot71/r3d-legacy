#include "r3d.h"

#include "./renderer.hpp"

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
