#include <stddef.h>
#include <r3d.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

Texture2D loadGroundTexture(void)
{
    Image image = GenImageChecked(128, 128, 8, 8,
        (Color) { 0, 128, 0, 255 },
        (Color) { 0, 32, 0, 255 }
    );
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

int main(void)
{
    InitWindow(800, 600, "R3D - Toon");
    SetTargetFPS(60);

    Texture texGround = loadGroundTexture();

    R3D_Init();

    R3D_MaterialConfig config = R3D_CreateMaterialConfig(
        R3D_DIFFUSE_TOON, R3D_SPECULAR_TOON,
        R3D_BLEND_ALPHA, R3D_CULL_BACK,
        R3D_MATERIAL_FLAG_RECEIVE_SHADOW
      | R3D_MATERIAL_FLAG_VERTEX_COLOR);

    R3D_SetDefaultMaterialConfig(config);

    R3D_Model ground = R3D_LoadModelFromMesh(GenMeshPlane(10, 10, 1, 1));
    R3D_SetMapAlbedo(&ground, 0, &texGround, WHITE);
    ground.shadow = R3D_CAST_OFF;

    // Bat model: https://sketchfab.com/3d-models/cartoon-bat-level-3-a11109f937bb4dde90f7484b3bdbe620
    R3D_Model bat = R3D_LoadModel(R3D_ASSETS_PATH "bat.glb");
    R3D_SetMapRoughness(&bat, 0, NULL, 1.0f);
    R3D_SetMapMetalness(&bat, 0, NULL, 0.0f);

    R3D_Light dirLight = R3D_CreateLight(R3D_DIRLIGHT, 4096);
    R3D_SetLightPosition(dirLight, (Vector3) { 0, 10, 10 });
    R3D_SetLightTarget(dirLight, (Vector3) { 0, 0, 0});
    R3D_SetLightActive(dirLight, true);

    Camera3D camera = {
        .position = (Vector3) { -7, 7, -7 },
        .target = (Vector3) { 0, 0.5, 0 },
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
                R3D_Draw(&ground);
                R3D_Draw(&bat);
            R3D_End();
            DrawText("Model by Builder123YT - (link in code)", 10, 10, 20, WHITE);
        EndDrawing();
    }

    R3D_UnloadModel(&ground);
    R3D_UnloadModel(&bat);

    UnloadTexture(texGround);

    R3D_Close();
    CloseWindow();

    return 0;
}
