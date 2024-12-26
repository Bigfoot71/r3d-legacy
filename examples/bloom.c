#include "raylib.h"
#include <stddef.h>
#include <r3d.h>

#include <raymath.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

Vector3 cubePosition(int x, int z)
{
    return (Vector3) { x * 4.0f, 0.5f, z * 4.0f };
}

void addLights(void)
{
    for (int z = -1; z <= 1; z++) {
        for (int x = -1; x <= 1; x++) {
            R3D_Light light = R3D_CreateLight(R3D_SPOTLIGHT, 0);
            R3D_SetLightPosition(light, cubePosition(x, z));
            R3D_SetLightColor(light, (Color) { 255, 0, 0 });
        }
    }
}

int main(void)
{
    InitWindow(800, 600, "R3D - Bloom");
    SetTargetFPS(60);

    R3D_Init();

    R3D_SetEnvBloomMode(R3D_BLOOM_ADDITIVE);
    R3D_SetEnvBloomIntensity(0.5f);

    R3D_Skybox sky = R3D_LoadSkybox(R3D_ASSETS_PATH "skybox_outdoor.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_SetEnvWorldSkybox(&sky);

    R3D_Model ground = R3D_LoadModelFromMesh(GenMeshPlane(100, 100, 1, 1));
    R3D_SetMapAlbedo(&ground, 0, NULL, GRAY);
    R3D_SetMapRoughness(&ground, 0, NULL, 0.2f);

    R3D_MaterialConfig cubeConfig = R3D_CreateMaterialConfig(
        R3D_DIFFUSE_BURLEY, R3D_SPECULAR_SCHLICK_GGX,
        R3D_BLEND_ALPHA, R3D_CULL_BACK,
        R3D_MATERIAL_FLAG_MAP_EMISSION);

    R3D_Model cube = R3D_LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    R3D_SetMapAlbedo(&cube, 0, NULL, (Color) { 255, 0, 0, 255 });
    R3D_SetMapEmission(&cube, 0, R3D_GetDefaultTextureWhite(), 5.0f, (Color) { 255, 0, 0, 255 });
    R3D_SetMaterialConfig(&cube, 0, cubeConfig);

    addLights();
    bool lighting = false;

    Camera3D camera = {
        .position = (Vector3) { -10, 10, -10 },
        .target = (Vector3) { 0, 0, 0 },
        .up = { 0, 1, 0 },
        .fovy = 60.0f
    };

    DisableCursor();

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);

        BeginDrawing();

            ClearBackground(BLACK);

            R3D_Begin(camera);

                R3D_DrawModel(&ground);

                for (int z = -1; z <= 1; z++) {
                    for (int x = -1; x <= 1; x++) {
                        R3D_DrawModelEx(&cube, cubePosition(x, z), 1.0f);
                    }
                }

                int intensity = IsKeyDown(KEY_V) - IsKeyDown(KEY_C);
                R3D_SetEnvBloomIntensity(Clamp(R3D_GetEnvBloomIntensity() + intensity * 0.01f, 0.0, 1.0));

                if (IsKeyPressed(KEY_L)) {
                    for (int light = 0; light < 6; light++) {
                        R3D_ToggleLight(light);
                    }
                    lighting = !lighting;
                }

            R3D_End();

            //DrawFPS(10, 10);
            DrawText(TextFormat("Intensity (C-V): %.2f", R3D_GetEnvBloomIntensity()), 10, 10, 20, BLACK);
            DrawText(TextFormat("Lights (L): %s", lighting ? "Enabled" : "Disabled" ), 10, 30, 20, BLACK);

        EndDrawing();
    }

    R3D_UnloadModel(&ground);
    R3D_UnloadModel(&cube);

    R3D_UnloadSkybox(&sky);
    R3D_Close();

    CloseWindow();
}
