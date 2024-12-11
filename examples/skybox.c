#include <stddef.h>
#include <r3d.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

int main(void)
{
    InitWindow(800, 600, "BRL - Basic example");
    SetTargetFPS(60);

    R3D_Init();

    R3D_SetDefaultMaterialConfig(R3D_CreateMaterialConfig(
        R3D_DIFFUSE_BURLEY, R3D_SPECULAR_SCHLICK_GGX,
        R3D_BLEND_ALPHA, R3D_CULL_BACK,
        R3D_MATERIAL_FLAG_RECEIVE_SHADOW |
        R3D_MATERIAL_FLAG_SKY_IBL
    ));

    R3D_Skybox sky = R3D_LoadSkybox(R3D_ASSETS_PATH "skybox_outdoor.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_SetEnvWorldSkybox(sky);

    R3D_Model sphere = R3D_LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));

    R3D_Light spotLight = R3D_CreateLight(R3D_DIRLIGHT, 0);
    R3D_SetLightDirection(spotLight, (Vector3) { 0, 0, -1 });
    R3D_SetLightActive(spotLight, true);

    Camera3D camera = {
        .position = (Vector3) { -15, -1, -15 },
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

                for (int x = -5; x <= 5; x += 2)
                {
                    for (int y = -5; y <= 5; y += 2)
                    {
                        R3D_SetMapMetalness(&sphere, 0, NULL, (5.0f+x)/10.0f);
                        R3D_SetMapRoughness(&sphere, 0, NULL, (5.0f+y)/10.0f);
                        R3D_SetMapAlbedo(&sphere, 0, NULL, ColorFromHSV((5.0f+x)*18, 1, 1));
                        R3D_DrawEx(&sphere, (Vector3) { (float)x, (float)y, 0 }, 1.0f);
                    }
                }

            R3D_End();

            DrawFPS(10, 10);

        EndDrawing();
    }

    R3D_UnloadModel(&sphere);
    R3D_UnloadSkybox(sky);
    R3D_Close();

    CloseWindow();

    return 0;
}
