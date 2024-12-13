#include <stddef.h>
#include <r3d.h>

int main(void)
{
    InitWindow(800, 600, "R3D - Point lights");
    SetTargetFPS(60);
    DisableCursor();

    R3D_Init();

    R3D_SetEnvWorldBackground(BLACK);
    R3D_SetEnvWorldAmbient(BLACK);

    R3D_Model ground = R3D_LoadModelFromMesh(GenMeshPlane(100, 100, 1, 1));
    R3D_SetMapAlbedo(&ground, 0, NULL, (Color) { 0, 0, 255, 255 });
    ground.transform.position = (Vector3) { 0, -15.5f, 0 };

    R3D_Model cube = R3D_LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    R3D_SetMapAlbedo(&cube, 0, NULL, (Color) { 255, 0, 0, 255 });
    R3D_SetMapRoughness(&cube, 0, NULL, 0.5f);
    R3D_SetMapMetalness(&cube, 0, NULL, 0.5f);

    R3D_Light spotLights[3 * 3 * 3];
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                R3D_Light light = R3D_CreateLight(R3D_OMNILIGHT, 1024);
                R3D_SetLightPosition(light, (Vector3){ x * 10.0f, y * 10.0f, z * 10.0f });
                R3D_SetLightShadowBias(light, 0.1f);
                R3D_SetLightRange(light, 16.0f);
                R3D_SetLightActive(light, true);

                spotLights[z * 3 * 3 + y * 3 + x] = light;
            }
        }
    }

    Camera3D camera = {
        .position = (Vector3) { 0, 0, 0 },
        .target = (Vector3) { 0, 0, -1 },
        .up = { 0, 1, 0 },
        .fovy = 60.0f
    };

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);

        BeginDrawing();

            ClearBackground(BLACK);

            R3D_Begin(camera);

                R3D_DrawModel(&ground);

                for (int z = -1; z <= 2; z++) {
                    for (int y = -1; y <= 2; y++) {
                        for (int x = -1; x <= 2; x++) {
                            R3D_DrawModelEx(&cube, (Vector3) {
                                x * 10.0f - 5.0f,
                                y * 10.0f - 5.0f,
                                z * 10.0f - 5.0f
                            }, 1.0f);
                        }
                    }
                }

                int sceneDrawCount, shadowDrawCount;
                R3D_GetDrawCallCount(&sceneDrawCount, &shadowDrawCount);

            R3D_End();

            BeginMode3D(camera);
                for (int i = 0; i < sizeof(spotLights) / sizeof(spotLights[0]); i++) {
                    DrawSphere(R3D_GetLightPosition(i), 0.2f, WHITE);
                }
            EndMode3D();

            DrawFPS(10, 10);

            DrawText(
                TextFormat("DRAWS: %i - SHADOW DRAWS: %i", sceneDrawCount, shadowDrawCount),
                10, 600 - 30, 20, LIME
            );

            const char MSG[] = "Here, some lights have no effect because we reach the limit of 8 lights per mesh";
            DrawText(MSG, 800 - MeasureText(MSG, 16) - 10, 13, 16, YELLOW);

        EndDrawing();
    }

    R3D_UnloadModel(&ground);
    R3D_UnloadModel(&cube);
    R3D_Close();

    CloseWindow();
}
