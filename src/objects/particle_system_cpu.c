/*
 * Copyright (c) 2024 Le Juez Victor
 * 
 * This software is provided "as-is", without any express or implied warranty. In no event 
 * will the authors be held liable for any damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose, including commercial 
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 * 
 *   1. The origin of this software must not be misrepresented; you must not claim that you 
 *   wrote the original software. If you use this software in a product, an acknowledgment 
 *   in the product documentation would be appreciated but is not required.
 * 
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 * 
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "r3d.h"

#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <raylib.h>
#include <raymath.h>

/* Helper Functions */

static float GetRandomFloat(void)
{
    static const float INV_65535 = 1.0f / 0xFFFF;
    return (float)GetRandomValue(0, 0xFFFF) * INV_65535;
}

static float GetRandomValueF(float min, float max)
{
    return min + GetRandomFloat() * (max - min);
}

static float Min3F(float a, float b, float c)
{
    float min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    return min;
}

static float Max3F(float a, float b, float c)
{
    float max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    return max;
}

/* Public API */

R3D_ParticleSystemCPU* R3D_LoadParticleEmitterCPU(const Mesh* mesh, const R3D_Material* material, int maxParticles)
{
    R3D_ParticleSystemCPU *system = RL_CALLOC(1, sizeof(R3D_ParticleSystemCPU));

    system->particles = RL_MALLOC(sizeof(R3D_Particle) * maxParticles);
    system->maxParticles = maxParticles;
    system->particleCount = 0;

    system->surface = (R3D_Surface) {
        .material = *material,
        .mesh = *mesh
    };

    system->position = (Vector3) { 0, 0, 0 };
    system->gravity = (Vector3) { 0, -9.81f, 0 };

    system->initialScale = Vector3One();
    system->scaleVariance = 0.0f;

    system->initialRotation = Vector3Zero();
    system->rotationVariance = Vector3Zero();

    system->initialColor = WHITE;
    system->colorVariance = BLANK;

    system->initialVelocity = (Vector3) { 0, 0, 0 };
    system->velocityVariance = Vector3Zero();

    system->initialAngularVelocity = Vector3Zero();
    system->angularVelocityVariance = Vector3Zero();

    system->lifetime = 1.0f;
    system->lifetimeVariance = 0.0f;

    system->emissionTimer = 0.0f;
    system->emissionRate = 1.0f;
    system->spreadAngle = 0.0f;

    system->aabb = (BoundingBox) {
        (Vector3) { -10.0f, -10.0f, -10.0f },
        (Vector3) { 10.0f, 10.0f, 10.0f }
    };

    system->scaleOverLifetime = NULL;
    system->speedOverLifetime = NULL;
    system->opacityOverLifetime = NULL;
    system->angularVelocityOverLifetime = NULL;

    system->shadow = R3D_CAST_OFF;
    system->billboard = R3D_BILLBOARD_ENABLED;
    system->layer = R3D_LAYER_1;

    system->autoEmission = true;

    return system;
}

void R3D_UnloadParticleEmitterCPU(R3D_ParticleSystemCPU* system)
{
    if (system) {
        RL_FREE(system->particles);
        RL_FREE(system);
    }
}

bool R3D_EmitParticleCPU(R3D_ParticleSystemCPU* system)
{
    if (system->particleCount >= system->maxParticles) {
        return false;
    }

    // Normalize the initial direction
    Vector3 direction = Vector3Normalize(system->initialVelocity);

    // Generate random angles
    float elevation = GetRandomValueF(0, system->spreadAngle * DEG2RAD);
    float azimuth = GetRandomValueF(0, 2.0f * PI);

    // Precompute trigonometric values for the cone
    float cosElevation = cosf(elevation);
    float sinElevation = sqrtf(1.0f - cosElevation * cosElevation); // Use the trigonometric identity
    float cosAzimuth = cosf(azimuth);
    float sinAzimuth = sinf(azimuth);

    // Calculate the vector within the cone (local coordinate system)
    Vector3 spreadDirection = {
        sinElevation * cosAzimuth,
        sinElevation * sinAzimuth,
        cosElevation
    };

    // Generate the local basis around 'direction'
    Vector3 arbitraryAxis = (fabsf(direction.y) > 0.9999f)
        ? (Vector3) { 0.0f, 0.0f, 1.0f }
        : (Vector3) { 1.0f, 0.0f, 0.0f };

    Vector3 binormal = Vector3Normalize(Vector3CrossProduct(arbitraryAxis, direction));
    Vector3 normal = Vector3CrossProduct(direction, binormal);

    // Transform 'spreadDirection' to the global coordinate system
    Vector3 velocity = {
        spreadDirection.x * binormal.x + spreadDirection.y * normal.x + spreadDirection.z * direction.x,
        spreadDirection.x * binormal.y + spreadDirection.y * normal.y + spreadDirection.z * direction.y,
        spreadDirection.x * binormal.z + spreadDirection.y * normal.z + spreadDirection.z * direction.z
    };

    // Scale the final velocity
    velocity = Vector3Scale(velocity, Vector3Length(system->initialVelocity));

    // Initialize particle
    R3D_Particle particle = { 0 };
    particle.lifetime = system->lifetime + GetRandomValueF(-system->lifetimeVariance, system->lifetimeVariance);
    particle.position = system->position;
    particle.rotation = (Vector3) {
        (system->initialRotation.x + GetRandomValueF(-system->rotationVariance.x, system->rotationVariance.x)) * DEG2RAD,
        (system->initialRotation.y + GetRandomValueF(-system->rotationVariance.y, system->rotationVariance.y)) * DEG2RAD,
        (system->initialRotation.z + GetRandomValueF(-system->rotationVariance.z, system->rotationVariance.z)) * DEG2RAD
    };
    particle.scale = particle.baseScale = Vector3AddValue(
        system->initialScale, GetRandomValueF(-system->scaleVariance, system->scaleVariance)
    );
    particle.velocity = particle.baseVelocity = (Vector3) {
        velocity.x + GetRandomValueF(-system->velocityVariance.x, system->velocityVariance.x),
        velocity.y + GetRandomValueF(-system->velocityVariance.y, system->velocityVariance.y),
        velocity.z + GetRandomValueF(-system->velocityVariance.z, system->velocityVariance.z)
    };
    particle.angularVelocity = particle.baseAngularVelocity = (Vector3) {
        system->initialAngularVelocity.x + GetRandomValueF(-system->angularVelocityVariance.x, system->angularVelocityVariance.x),
        system->initialAngularVelocity.y + GetRandomValueF(-system->angularVelocityVariance.y, system->angularVelocityVariance.y),
        system->initialAngularVelocity.z + GetRandomValueF(-system->angularVelocityVariance.z, system->angularVelocityVariance.z)
    };
    particle.color = (Color) {
        (unsigned char)(system->initialColor.r + GetRandomValue(-system->colorVariance.r, system->colorVariance.r)),
        (unsigned char)(system->initialColor.g + GetRandomValue(-system->colorVariance.g, system->colorVariance.g)),
        (unsigned char)(system->initialColor.g + GetRandomValue(-system->colorVariance.b, system->colorVariance.b)),
        (unsigned char)(system->initialColor.a + GetRandomValue(-system->colorVariance.a, system->colorVariance.a))
    };
    particle.baseOpacity = particle.color.a;

    // Adding the particle to the system
    system->particles[system->particleCount++] = particle;

    return true;
}

void R3D_UpdateParticleEmitterCPU(R3D_ParticleSystemCPU* system, float deltaTime)
{
    system->emissionTimer -= deltaTime;

    if (system->emissionRate > 0.0f) {
        while (system->emissionTimer <= 0.0f) {
            R3D_EmitParticleCPU(system);
            system->emissionTimer += 1.0f / system->emissionRate;
            if (system->emissionRate <= 0.0f) {
                break;
            }
        }
    }

    for (int i = system->particleCount - 1; i >= 0; i--) {
        R3D_Particle *particle = &system->particles[i];

        particle->lifetime -= deltaTime;
        if (particle->lifetime <= 0.0f) {
            particle = &system->particles[--system->particleCount];
            continue;
        }

        float t = 1.0f - (particle->lifetime / system->lifetime);

        if (system->scaleOverLifetime) {
            float scale = R3D_EvaluateCurve(system->scaleOverLifetime, t);
            particle->scale.x = particle->baseScale.x *  scale;
            particle->scale.y = particle->baseScale.y *  scale;
            particle->scale.z = particle->baseScale.z *  scale;
        }

        if (system->opacityOverLifetime) {
            float scale = R3D_EvaluateCurve(system->opacityOverLifetime, t);
            particle->color.a = Clamp(particle->baseOpacity * scale, 0.0f, 255.0f);
        }

        if (system->speedOverLifetime) {
            float scale = R3D_EvaluateCurve(system->speedOverLifetime, t);
            particle->velocity.x = particle->baseVelocity.x * scale;
            particle->velocity.y = particle->baseVelocity.y * scale;
            particle->velocity.z = particle->baseVelocity.z * scale;
        }

        if (system->angularVelocityOverLifetime) {
            float scale = R3D_EvaluateCurve(system->angularVelocityOverLifetime, t);
            particle->angularVelocity.x = particle->baseAngularVelocity.x * scale;
            particle->angularVelocity.y = particle->baseAngularVelocity.y * scale;
            particle->angularVelocity.z = particle->baseAngularVelocity.z * scale;
        }

        particle->rotation.x += particle->angularVelocity.x * deltaTime * DEG2RAD;
        particle->rotation.y += particle->angularVelocity.y * deltaTime * DEG2RAD;
        particle->rotation.z += particle->angularVelocity.z * deltaTime * DEG2RAD;

        particle->position.x += particle->velocity.x * deltaTime;
        particle->position.y += particle->velocity.y * deltaTime;
        particle->position.z += particle->velocity.z * deltaTime;

        particle->velocity.x += system->gravity.x * deltaTime;
        particle->velocity.y += system->gravity.y * deltaTime;
        particle->velocity.z += system->gravity.z * deltaTime;
    }
}

void R3D_UpdateParticleEmitterCPUAABB(R3D_ParticleSystemCPU* system)
{
    // Initialize the AABB with extreme values (max for max bounds, min for min bounds)
    Vector3 aabbMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    Vector3 aabbMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    // Loop over all particles in the emitter (considering the particle capacity)
    for (int i = 0; i < system->maxParticles; i++) {
        // Emit a particle for the current iteration
        R3D_EmitParticleCPU(system);

        // Get the current particle from the emitter
        const R3D_Particle *particle = &system->particles[i];

        // Calculate the position of the particle at half its lifetime (intermediate position)
        float halfLifetime = particle->lifetime * 0.5f;
        Vector3 midPosition = {
            particle->position.x + particle->velocity.x * halfLifetime + 0.5f * system->gravity.x * halfLifetime * halfLifetime,
            particle->position.y + particle->velocity.y * halfLifetime + 0.5f * system->gravity.y * halfLifetime * halfLifetime,
            particle->position.z + particle->velocity.z * halfLifetime + 0.5f * system->gravity.z * halfLifetime * halfLifetime
        };

        // Calculate the position of the particle at the end of its lifetime (final position)
        Vector3 futurePosition = {
            particle->position.x + particle->velocity.x * particle->lifetime + 0.5f * system->gravity.x * particle->lifetime * particle->lifetime,
            particle->position.y + particle->velocity.y * particle->lifetime + 0.5f * system->gravity.y * particle->lifetime * particle->lifetime,
            particle->position.z + particle->velocity.z * particle->lifetime + 0.5f * system->gravity.z * particle->lifetime * particle->lifetime
        };

        // Expand the AABB by comparing the current min and max with the calculated positions
        // We include both the intermediate (halfway) and final (end of lifetime) positions
        aabbMin.x = Min3F(aabbMin.x, midPosition.x, futurePosition.x);
        aabbMin.y = Min3F(aabbMin.y, midPosition.y, futurePosition.y);
        aabbMin.z = Min3F(aabbMin.z, midPosition.z, futurePosition.z);

        aabbMax.x = Max3F(aabbMax.x, midPosition.x, futurePosition.x);
        aabbMax.y = Max3F(aabbMax.y, midPosition.y, futurePosition.y);
        aabbMax.z = Max3F(aabbMax.z, midPosition.z, futurePosition.z);
    }

    // Clear any previous state or data in the emitter
    system->particleCount = 0;

    // Update the particle system's AABB with the calculated bounds
    system->aabb = (BoundingBox) { aabbMin, aabbMax };
}
