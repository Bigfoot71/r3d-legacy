#include "raylib.h"
#include <stddef.h>
#include <math.h>
#include <r3d.h>

int getLayers(int from, int to)
{
    int layers = 0;
    for (int i = from; i < to; i++) {
        layers |= 1 << i;
    }
    return layers;
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "R3D - Resize expanded");
    SetTargetFPS(60);

    R3D_Init();
    R3D_SetActiveLayers(getLayers(0, 7));

    R3D_Model sphere = R3D_LoadModelFromMesh(GenMeshSphere(1.0f, 32, 64));

    R3D_Light dirLight = R3D_CreateLight(R3D_DIRLIGHT, 4096);
    R3D_SetLightPosition(dirLight, (Vector3) { 0, 0, 10 });
    R3D_SetLightTarget(dirLight, (Vector3) { 0, 0, 0});
    R3D_SetLightLayers(dirLight, getLayers(0, 7));
    R3D_SetLightActive(dirLight, true);

    Camera3D camera = {
        .position = (Vector3) { 0, 0, 10 },
        .target = (Vector3) { 0, 0, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    int animFrame = 0;

    while (!WindowShouldClose())
    {
        switch (GetKeyPressed()) {
            case KEY_ONE: R3D_ToggleLightLayer(dirLight, R3D_LAYER_0); break;
            case KEY_TWO: R3D_ToggleLightLayer(dirLight, R3D_LAYER_1); break;
            case KEY_THREE: R3D_ToggleLightLayer(dirLight, R3D_LAYER_2); break;
            case KEY_FOUR: R3D_ToggleLightLayer(dirLight, R3D_LAYER_3); break;
            case KEY_FIVE: R3D_ToggleLightLayer(dirLight, R3D_LAYER_4); break;
            case KEY_SIX: R3D_ToggleLightLayer(dirLight, R3D_LAYER_5); break;
            case KEY_SEVEN: R3D_ToggleLightLayer(dirLight, R3D_LAYER_6); break;
            default: break;
        }

        BeginDrawing();
            ClearBackground(BLACK);
            R3D_Begin(camera);
                for (int x = -3; x <= 3; x++) {
                    sphere.layer = (R3D_Layer)powf(2, (x + 3));
                    R3D_SetMapAlbedo(&sphere, 0, NULL, ColorFromHSV((x + 3) / 6.0f * 360, 1.0f, 1.0f));
                    R3D_DrawModelEx(&sphere, (Vector3) { x * 2.0f, 0, 0 }, 1.0f);
                }
            R3D_End();
            DrawFPS(10, 10);
        EndDrawing();
    }

    R3D_UnloadModel(&sphere);
    R3D_Close();

    CloseWindow();

    return 0;
}
