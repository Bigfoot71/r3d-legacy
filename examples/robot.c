#include <stddef.h>
#include <r3d.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

Texture2D loadGroundTexture(void)
{
    Image image = GenImageChecked(128, 128, 8, 8, BLUE, SKYBLUE);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

int main(void)
{
    InitWindow(800, 600, "R3D - Robot");
    SetTargetFPS(60);

    Texture texGround = loadGroundTexture();

    R3D_Init();

    R3D_Skybox sky = R3D_LoadSkybox(R3D_ASSETS_PATH "skybox_indoor.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_SetEnvWorldSkybox(&sky);

    R3D_Model ground = R3D_LoadModelFromMesh(GenMeshPlane(10, 10, 1, 1));
    R3D_SetMapAlbedo(&ground, 0, &texGround, WHITE);
    ground.shadow = R3D_CAST_OFF;

    R3D_Model robot = R3D_LoadModel(R3D_ASSETS_PATH "robot.glb");
    R3D_LoadModelAnimations(&robot, R3D_ASSETS_PATH "robot.glb");
    for (int i = 0; i < R3D_GetSurfaceCount(&robot); i++) {
        R3D_SetMapMetalness(&robot, i, NULL, 1.0f);
        R3D_SetMapRoughness(&robot, i, NULL, 0.5f);
    }

    R3D_Light dirLight = R3D_CreateLight(R3D_DIRLIGHT, 4096);
    R3D_SetLightPosition(dirLight, (Vector3) { 0, 5, 5 });
    R3D_SetLightTarget(dirLight, (Vector3) { 0, 0, 0});
    R3D_SetLightActive(dirLight, true);

    Camera3D camera = {
        .position = (Vector3) { -7, 7, -7 },
        .target = (Vector3) { 0, 1, 0 },
        .up = (Vector3) { 0, 1, 0 },
        .fovy = 60.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    int animFrame = 0;

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_ORBITAL);

        R3D_UpdateModelAnimation(&robot, "Robot_Dance", animFrame++);

        BeginDrawing();
            ClearBackground(BLACK);
            R3D_Begin(camera);
                R3D_DrawModel(&ground);
                R3D_DrawModel(&robot);
            R3D_End();
            DrawFPS(10, 10);
        EndDrawing();
    }

    R3D_UnloadModel(&ground);
    R3D_UnloadModel(&robot);
    R3D_UnloadSkybox(&sky);

    UnloadTexture(texGround);

    R3D_Close();
    CloseWindow();

    return 0;
}
