#include <r3d.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

int main(void)
{
    InitWindow(800, 600, "R3D - Particles");
    SetTargetFPS(60);

    Mesh sphere = GenMeshSphere(0.1f, 16, 32);

    R3D_Init();

    R3D_SetEnvWorldBackground(BLACK);
    R3D_SetEnvWorldAmbient(BLACK);

    R3D_MaterialConfig config = R3D_CreateMaterialConfig(
        R3D_DIFFUSE_BURLEY, R3D_SPECULAR_SCHLICK_GGX,
        R3D_BLEND_ADDITIVE, R3D_CULL_BACK,
        R3D_MATERIAL_FLAG_MAP_EMISSION
    );

    R3D_Material material = R3D_CreateMaterial(config);
    material.emission.texture = *R3D_GetDefaultTextureWhite();
    material.emission.color = (Color) { 255, 0, 0 };
    material.emission.energy = 1.0f;

    R3D_InterpolationCurve curve = R3D_LoadInterpolationCurve(3);
    R3D_AddKeyframe(&curve, 0.0f, 0.0f);
    R3D_AddKeyframe(&curve, 0.5f, 1.0f);
    R3D_AddKeyframe(&curve, 1.0f, 0.0f);

    R3D_ParticleSystemCPU *particles = R3D_LoadParticleEmitterCPU(&sphere, &material, 512);
    particles->initialVelocity = (Vector3) { 0, 10.0f, 0 };
    particles->billboard = R3D_BILLBOARD_DISABLED;
    particles->scaleOverLifetime = &curve;
    particles->spreadAngle = 45.0f;
    particles->initialColor = RED;
    particles->emissionRate = 500;
    particles->lifetime = 2.0f;

    R3D_UpdateParticleEmitterCPUAABB(particles);

    R3D_Light dirLight = R3D_CreateLight(R3D_DIRLIGHT, 0);
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

        R3D_UpdateParticleEmitterCPU(particles, GetFrameTime());

        BeginDrawing();
            ClearBackground(BLACK);
            R3D_Begin(camera);
                R3D_DrawParticleSystemCPU(particles);
            R3D_End();
            BeginMode3D(camera);
                DrawBoundingBox(particles->aabb, GREEN);
            EndMode3D();
            DrawFPS(10, 10);
        EndDrawing();
    }

    R3D_UnloadParticleEmitterCPU(particles);
    R3D_Close();

    UnloadMesh(sphere);
    CloseWindow();

    return 0;
}
