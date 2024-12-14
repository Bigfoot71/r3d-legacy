#include <r3d.h>
#include <raymath.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

Texture loadTexture(const char* fileName)
{
    Texture texture = LoadTexture(fileName);

    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_ANISOTROPIC_4X);

    return texture;
}

int main(void)
{
    InitWindow(800, 600, "R3D - PBR demo");

    Camera camera = {
        .position = (Vector3) { 0.0f, 0.0f, 4.0f },
        .target = (Vector3) { 0.0f, 0.0f, 0.0f },
        .up = (Vector3) { 0.0f, 1.0f, 0.0f },
        .fovy = 45.0f
    };

    Texture albedo = loadTexture(R3D_ASSETS_PATH "pbr/albedo.png");
    Texture normal = loadTexture(R3D_ASSETS_PATH "pbr/normal.png");
    Texture metalness = loadTexture(R3D_ASSETS_PATH "pbr/metallic.png");
    Texture roughness = loadTexture(R3D_ASSETS_PATH "pbr/roughness.png");
    Texture occlusion = loadTexture(R3D_ASSETS_PATH "pbr/occlusion.png");

    R3D_Init();

    R3D_Skybox sky = R3D_LoadSkybox(R3D_ASSETS_PATH "skybox_indoor.png", CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_SetEnvWorldSkybox(&sky);

    R3D_MaterialConfig config = R3D_CreateMaterialConfig(
        R3D_DIFFUSE_BURLEY, R3D_SPECULAR_SCHLICK_GGX,
        R3D_BLEND_ALPHA, R3D_CULL_BACK,
        R3D_MATERIAL_FLAG_MAP_AO |
        R3D_MATERIAL_FLAG_MAP_NORMAL |
        R3D_MATERIAL_FLAG_SKY_IBL
    );

    R3D_SetDefaultMaterialConfig(config);

    R3D_Light light = R3D_CreateLight(R3D_OMNILIGHT, 0);
    R3D_SetLightPosition(light, (Vector3) { 0, 0, 8 });
    R3D_SetLightActive(light, true);

    R3D_Model sphere = R3D_LoadModelFromMesh(GenMeshSphere(1.0f, 32, 64));
    R3D_GenTangents(&sphere);

    R3D_SetMapAlbedo(&sphere, 0, &albedo, WHITE);
    R3D_SetMapNormal(&sphere, 0, &normal);
    R3D_SetMapMetalness(&sphere, 0, &metalness, 2.0f);
    R3D_SetMapRoughness(&sphere, 0, &roughness, 1.0f);
    R3D_SetMapAO(&sphere, 0, &occlusion, 0.5f);

    float modelScale = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        modelScale = Clamp(modelScale + GetMouseWheelMove() * 0.1, 0.25f, 2.5f);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            sphere.transform.rotation = QuaternionMultiply(QuaternionFromEuler(
                (GetMouseDelta().y * 0.0025f) / modelScale,
                (GetMouseDelta().x * 0.0025f) / modelScale,
                0.0f
            ), sphere.transform.rotation);
        }

        BeginDrawing();

            ClearBackground(DARKGRAY);

            R3D_Begin(camera);
                R3D_DrawModelEx(&sphere, Vector3Zero(), modelScale);
            R3D_End();

        EndDrawing();
    }

    UnloadTexture(albedo);
    UnloadTexture(normal);
    UnloadTexture(metalness);
    UnloadTexture(roughness);
    UnloadTexture(occlusion);

    R3D_UnloadModel(&sphere);
    R3D_UnloadSkybox(&sky);
    R3D_Close();

    CloseWindow();

    return 0;
}
