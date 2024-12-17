#include "raylib.h"
#include "raymath.h"
#include <stddef.h>
#include <r3d.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

int main(void)
{
    InitWindow(800, 600, "R3D - Sprite");
    SetTargetFPS(60);

    Texture texSprite = LoadTexture(R3D_ASSETS_PATH "spritesheet.png");

    R3D_Init();

    R3D_Skybox sky = R3D_LoadSkybox(R3D_ASSETS_PATH "skybox_outdoor.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_SetEnvWorldSkybox(&sky);

    R3D_Sprite sprite = R3D_CreateSprite(texSprite, 5, 5);

    sprite.material.config = R3D_CreateMaterialConfig(
        R3D_DIFFUSE_UNSHADED, R3D_SPECULAR_DISABLED,
        R3D_BLEND_ALPHA, R3D_CULL_BACK,
        0);

    Camera3D camera = {
        .position = (Vector3) { -7, 5, -7 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    int animFrame = 0;

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        R3D_UpdateSprite(&sprite, 30.0f * GetFrameTime());

        BeginDrawing();
            ClearBackground(BLACK);
            R3D_Begin(camera);
                R3D_DrawSpriteEx(&sprite, Vector3Zero(), 10.0f);
            R3D_End();
            DrawFPS(10, 10);
        EndDrawing();
    }

    R3D_UnloadSkybox(&sky);
    R3D_Close();

    UnloadTexture(texSprite);
    CloseWindow();

    return 0;
}
