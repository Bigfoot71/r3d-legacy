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

#ifndef R3D_PARTICLE_EMITTER_CPU_HPP
#define R3D_PARTICLE_EMITTER_CPU_HPP

#include "../detail/RandomGenerator.hpp"
#include <raylib.h>

namespace r3d {

struct Particle
{
    Vector3 position;
    Vector3 scale;
    Vector3 rotation;           ///< en radian euler
    Color color;                ///< color modulation

    Vector3 velocity;
    Vector3 angularVelocity;    ///< en radian euler

    float lifetime;

    Vector3 baseScale;
    Vector3 baseVelocity;
    Vector3 baseAngularVelocity;
    uint8_t baseOpacity;

    Particle() = default;

    Particle(
        Vector3 position,
        Vector3 scale,
        Vector3 rotation,
        Color color,
        Vector3 velocity,
        Vector3 angularVelocity,
        float lifetime
    )
        : position(position)
        , scale(scale)
        , rotation(rotation)
        , color(color)
        , velocity(velocity)
        , angularVelocity(angularVelocity)
        , lifetime(lifetime)
        , baseScale(scale)
        , baseVelocity(velocity)
        , baseAngularVelocity(angularVelocity)
        , baseOpacity(color.a)
    { }
};

class ParticleEmitterCPU
{
public:
    RandomGenerator gen;
    float emissionTimer;

public:
    ParticleEmitterCPU(int maxParticles)
        : mParticles(new Particle[maxParticles]{})
        , mParticleCount(0)
        , mMaxParticles(maxParticles)
        , gen(GetTime())
        , emissionTimer(0)
    { }

    ~ParticleEmitterCPU() {
        delete[] mParticles;
    }

    Particle& operator[](int i) {
        return mParticles[i];
    }

    const Particle& operator[](int i) const {
        return mParticles[i];
    }

    Particle* begin() {
        return mParticles;
    }

    Particle* end() {
        return mParticles + mParticleCount;
    }

    void add(const Particle& particle) {
        mParticles[mParticleCount++] = particle;
    }

    void remove(Particle& particle) {
        if (mParticleCount > 0) {
            particle = mParticles[--mParticleCount];
        }
    }

    void clear() {
        mParticleCount = 0;
    }

    int count() const {
        return mParticleCount;
    }

    int capacity() const {
        return mMaxParticles;
    }

private:
    Particle* mParticles;
    int mParticleCount;
    int mMaxParticles;
};

} // namespace r3d

#endif // R3D_PARTICLE_EMITTER_CPU_HPP
