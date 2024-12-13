#include <r3d.h>

#ifndef R3D_ASSETS_PATH
#   define R3D_ASSETS_PATH "assets/"
#endif

int main(void)
{
    InitWindow(800, 600, "R3D - Particles");
    SetTargetFPS(60);

    R3D_Init();

    Mesh sphere = GenMeshSphere(0.1f, 16, 32);
    R3D_Material material = R3D_CreateMaterial(R3D_GetDefaultMaterialConfig());

    R3D_InterpolationCurve curve = R3D_LoadInterpolationCurve(3);
    R3D_AddKeyframe(&curve, 0.0f, 0.0f);
    R3D_AddKeyframe(&curve, 0.5f, 1.0f);
    R3D_AddKeyframe(&curve, 1.0f, 0.0f);

    R3D_ParticleSystemCPU *particles = R3D_LoadParticleEmitterCPU(&sphere, &material, 128);
    particles->initialVelocity = (Vector3) { 0, 10.0f, 0 };
    particles->scaleOverLifetime = &curve;
    particles->spreadAngle = 45.0f;
    particles->initialColor = RED;
    particles->emissionRate = 100;
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
    CloseWindow();

    return 0;
}
