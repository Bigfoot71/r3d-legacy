#include "r3d.h"

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>

/* Public API */

R3D_Sprite R3D_CreateSprite(Texture2D texture, int xFrameCount, int yFrameCount)
{
    R3D_Sprite sprite = { 0 };

    sprite.transform = R3D_CreateTransformIdentity(NULL);

    sprite.material = R3D_CreateMaterial(R3D_GetDefaultMaterialConfig());
    sprite.material.albedo.texture = texture;

    sprite.currentFrame = 0;
    sprite.frameSize = (Vector2) {
        (float)texture.width / xFrameCount,
        (float)texture.height / yFrameCount,
    };

    sprite.xFrameCount = xFrameCount;
    sprite.yFrameCount = yFrameCount;

    sprite.shadow = R3D_CAST_OFF;
    sprite.billboard = R3D_BILLBOARD_Y_AXIS;
    sprite.layer = R3D_LAYER_0;

    return sprite;
}

void R3D_UpdateSprite(R3D_Sprite* sprite, float speed)
{
    R3D_UpdateSpriteEx(sprite, 0, sprite->xFrameCount * sprite->yFrameCount, speed);
}

void R3D_UpdateSpriteEx(R3D_Sprite* sprite, int firstFrame, int lastFrame, float speed)
{
    sprite->currentFrame = Wrap(sprite->currentFrame + speed, firstFrame, lastFrame);

    Rectangle rect = R3D_GetCurrentSpriteFrameRect(sprite);

    float wInv = 1.0f / sprite->material.albedo.texture.width;
    float hInv = 1.0f / sprite->material.albedo.texture.height;

    sprite->material.uv.offset = (Vector2) { rect.x * wInv, rect.y * hInv };
    sprite->material.uv.scale = (Vector2) { rect.width * wInv, rect.height * hInv };
}

Vector2 R3D_GetCurrentSpriteFrameCoord(const R3D_Sprite* sprite)
{
    int xFrame = (int)(sprite->currentFrame) % sprite->xFrameCount;
    int yFrame = (int)(sprite->currentFrame) / sprite->yFrameCount;
    return Vector2Multiply((Vector2) { (float)xFrame, (float)yFrame }, sprite->frameSize);
}

Rectangle R3D_GetCurrentSpriteFrameRect(const R3D_Sprite* sprite)
{
    Vector2 coord = R3D_GetCurrentSpriteFrameCoord(sprite);

    return (Rectangle) {
        coord.x, coord.y,
        sprite->frameSize.x,
        sprite->frameSize.y
    };
}
