#include <stddef.h>
#include <r3d.h>

int main(void)
{
    InitWindow(800, 600, "R3D - Bloom");
    SetTargetFPS(60);

    R3D_Init();

    R3D_SetEnvBloomMode(R3D_BLOOM_ADDITIVE);
    R3D_SetEnvBloomIntensity(0.2f);
    R3D_SetEnvBloomHDRThreshold(0.1f);

    R3D_Model ground = R3D_LoadModelFromMesh(GenMeshPlane(100, 100, 1, 1));
    R3D_SetMapAlbedo(&ground, 0, NULL, GRAY);

    R3D_MaterialConfig cubeConfig = R3D_CreateMaterialConfig(
        R3D_DIFFUSE_BURLEY, R3D_SPECULAR_SCHLICK_GGX,
        R3D_BLEND_ALPHA, R3D_CULL_BACK,
        R3D_MATERIAL_FLAG_MAP_EMISSION);

    R3D_Model cube = R3D_LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    R3D_SetMapAlbedo(&cube, 0, NULL, (Color) { 255, 0, 0, 255 });
    R3D_SetMapEmission(&cube, 0, R3D_GetDefaultTextureWhite(), 10.0f, (Color) { 255, 0, 0, 255 });
    R3D_SetMaterialConfig(&cube, 0, cubeConfig);

    Camera3D camera = {
        .position = (Vector3) { -10, 10, -10 },
        .target = (Vector3) { 0, 0, 0 },
        .up = { 0, 1, 0 },
        .fovy = 60.0f
    };

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();

            ClearBackground(BLACK);

            R3D_Begin(camera);

                R3D_Draw(&ground);

                for (int z = -1; z <= 1; z++) {
                    for (int x = -1; x <= 1; x++) {
                        R3D_DrawEx(&cube, (Vector3) {
                            x * 4.0f,
                            0.5f,
                            z * 4.0f
                        }, 1.0f);
                    }
                }

            R3D_End();

            DrawFPS(10, 10);

        EndDrawing();
    }

    R3D_UnloadModel(&ground);
    R3D_UnloadModel(&cube);
    R3D_Close();

    CloseWindow();
}
