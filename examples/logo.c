#include <r3d.h>

#include <raymath.h>
#include <stddef.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

Texture2D genLogoR3D(void)
{
    Image image = GenImageColor(512, 512, RAYWHITE);

    ImageDrawRectangleLines(&image, (Rectangle) { 0, 0, 512, 512 }, 32, BLACK);
    ImageDrawText(&image, "r3d", 46, 512 - 64 - 32, 64, BLACK);
    ImageFlipVertical(&image);

    Texture2D texture = LoadTextureFromImage(image);

    UnloadImage(image);

    return texture;
}

int main(void)
{
    InitWindow(800, 600, "R3D - Toon");
    SetTargetFPS(60);

    Texture2D texR3D = genLogoR3D();

    R3D_Init();

    R3D_SetEnvWorldBackground(GRAY);
    R3D_SetEnvWorldAmbient(GRAY);

    R3D_Skybox sky = R3D_LoadSkybox(R3D_ASSETS_PATH "skybox_outdoor.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_SetEnvWorldSkybox(&sky);

    R3D_Model cube = R3D_LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    R3D_SetMapRoughness(&cube, 0, NULL, 0.225f);
    R3D_SetMapMetalness(&cube, 0, NULL, 1.0f);
    R3D_SetMapAlbedo(&cube, 0, &texR3D, WHITE);

    Camera3D camera = {
        .position = (Vector3) { -2, 1, -2 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    int animFrame = 0;

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();
            ClearBackground(BLACK);
            R3D_Begin(camera);
                R3D_DrawModel(&cube);
            R3D_End();
        EndDrawing();
    }

    R3D_UnloadModel(&cube);
    R3D_UnloadSkybox(&sky);
    R3D_Close();

    UnloadTexture(texR3D);
    CloseWindow();

    return 0;
}
