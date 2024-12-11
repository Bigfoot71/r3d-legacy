#include "raylib.h"
#include <stddef.h>
#include <r3d.h>

int main(void)
{
    InitWindow(800, 600, "R3D - Custom target");
    SetTargetFPS(60);

    R3D_Init();

    R3D_Model sphere = R3D_LoadModelFromMesh(GenMeshSphere(1.0f, 32, 64));

    R3D_Light dirLight = R3D_CreateLight(R3D_DIRLIGHT, 4096);
    R3D_SetLightPosition(dirLight, (Vector3) { 0, 0, -10 });
    R3D_SetLightTarget(dirLight, (Vector3) { 0, 0, 0});
    R3D_SetLightActive(dirLight, true);

    Camera3D camera = {
        .position = (Vector3) { 0, 0, -10 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    RenderTexture target = LoadRenderTexture(800, 600);
    R3D_SetRenderTarget(&target);

    int animFrame = 0;

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        BeginDrawing();

            ClearBackground(BLACK);

            R3D_Begin(camera);
                for (int x = -3; x <= 3; x++) {
                    R3D_SetMapAlbedo(&sphere, 0, NULL, ColorFromHSV((x + 3) / 6.0f * 360, 1.0f, 1.0f));
                    R3D_DrawEx(&sphere, (Vector3) { x * 2.0f, 0, 0 }, 1.0f);
                }
            R3D_End();

            DrawTexturePro(
                target.texture,
                (Rectangle) { 0, 0, 800, 600 },
                (Rectangle) { 400, 300, 400, 300 },
                (Vector2) { 200, 150 },
                45 * GetTime(),
                WHITE
            );

            DrawFPS(10, 10);

        EndDrawing();
    }

    R3D_UnloadModel(&sphere);
    R3D_Close();

    CloseWindow();

    return 0;
}
