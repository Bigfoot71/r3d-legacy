#include <stddef.h>
#include <r3d.h>

int main(void)
{
    InitWindow(800, 600, "R3D - Shadow maps");
    SetTargetFPS(60);
    DisableCursor();

    R3D_InitEx(0, 0, R3D_FLAG_DEBUG_SHADOW_MAP);

    R3D_SetEnvWorldBackground(BLACK);
    R3D_SetEnvWorldAmbient(BLACK);

    R3D_Model ground = R3D_LoadModelFromMesh(GenMeshPlane(100, 100, 1, 1));
    R3D_SetMapAlbedo(&ground, 0, NULL, BLUE);

    R3D_Model cube = R3D_LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

    R3D_Light dirLight = R3D_CreateLight(R3D_DIRLIGHT, 2048);
    R3D_SetLightPosition(dirLight, (Vector3) { 0, 1000, -1000 });
    R3D_SetLightTarget(dirLight, (Vector3) { 0, 0, 0 });
    R3D_SetLightActive(dirLight, true);

    R3D_Light spotLight = R3D_CreateLight(R3D_SPOTLIGHT, 2048);
    R3D_SetLightPosition(spotLight, (Vector3) { -5, 10, -10 });
    R3D_SetLightTarget(spotLight, (Vector3) { -5, 0, 0 });
    R3D_SetLightOuterCutOff(spotLight, 60.0f);
    R3D_SetLightInnerCutOff(spotLight, 45.0f);
    R3D_SetLightActive(spotLight, true);

    R3D_Light omniLight = R3D_CreateLight(R3D_OMNILIGHT, 2048);
    R3D_SetLightPosition(omniLight, (Vector3) { 5, 10, -10 });
    R3D_SetLightShadowBias(omniLight, 0.05f);
    R3D_SetLightActive(omniLight, true);

    Camera3D camera = {
        .position = (Vector3) { 0, 10, -10 },
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

                R3D_DrawModel(&ground);

                R3D_SetMapAlbedo(&cube, 0, NULL, (Color) { 255, 0, 0, 255 });
                R3D_DrawModelEx(&cube, (Vector3) { -5, 0.5f, 0 }, 1.0f);

                R3D_SetMapAlbedo(&cube, 0, NULL, (Color) { 0, 255, 0, 255 });
                R3D_DrawModelEx(&cube, (Vector3) { 0, 0.5f, 0 }, 1.0f);

                R3D_SetMapAlbedo(&cube, 0, NULL, (Color) { 0, 0, 255, 255 });
                R3D_DrawModelEx(&cube, (Vector3) { 5, 0.5f, 0 }, 1.0f);

            R3D_End();

            R3D_DrawShadowMap(dirLight, 0, 0, 64, 64, 0.01f, 10.0f);
            R3D_DrawShadowMap(spotLight, 64, 0, 64, 64, 0.01f, 10.0f);
            R3D_DrawShadowMap(omniLight, 128, 0, 64, 64, 0, 100.0f);

        EndDrawing();
    }

    R3D_UnloadModel(&ground);
    R3D_Close();

    CloseWindow();
}
