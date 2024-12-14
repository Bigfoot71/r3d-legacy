/**
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

#ifndef R3D_DETAIL_RANDOM_GENERATOR_HPP
#define R3D_DETAIL_RANDOM_GENERATOR_HPP

#include <cstdint>
#include <limits>
#include <array>

namespace r3d {

// pseudrandom generator xoshiro256**
// should be evaluated at compile time if the seed is hard coded
// compile time evaluation example: https://godbolt.org/z/qYnnY6qah
class RandomGenerator
{
public:
    static RandomGenerator& singleton() {
        static RandomGenerator gen(0x9E3779B97F4A7C15);
        return gen;
    }

public:
    constexpr RandomGenerator(uint64_t seed) noexcept
        : mState{
            splitmix64(seed),
            splitmix64(seed + 1),
            splitmix64(seed + 2),
            splitmix64(seed + 3)
        }
    { }

    constexpr void seed(uint64_t seed) noexcept {
        for (auto& s : mState) {
            s = splitmix64(seed++);
        }
    }

    template<typename T>
    constexpr T rand() noexcept {
        if constexpr (std::is_same_v<T, float>) {
            constexpr uint32_t mask = (1ULL << 23) - 1;
            constexpr float scale = 1.0f / (1ULL << 23);
            return (static_cast<uint32_t>(next() >> 32) & mask) * scale;
        }
        else if constexpr (std::is_same_v<T, double>) {
            constexpr uint64_t mask = (1ULL << 52) - 1;
            constexpr double scale = 1.0 / (1ULL << 52);
            return (next() & mask) * scale;
        }
        else if constexpr (std::is_integral_v<T>) {
            constexpr int shift = (sizeof(uint64_t) - sizeof(T)) * 8;
            if constexpr (sizeof(T) == 8) {
                return static_cast<T>(next());
            } else {
                return static_cast<T>(next() >> shift);
            }
        }
        static_assert(std::is_arithmetic_v<T>, "Type must be arithmetic");
        return T{}; // Never reached, just to satisfy compiler
    }

    template <typename T>
    constexpr T rand(T min, T max) noexcept {
        if (min >= max) std::swap(min, max);
        T n = rand<T>();
        if constexpr (std::is_integral_v<T>) {
            n /= static_cast<T>(std::numeric_limits<T>::max());
        }
        return min + n * (max - min);
    }

    /* This is the jump function for the generator. It is equivalent
    to 2^128 calls to next(); it can be used to generate 2^128
    non-overlapping subsequences for parallel computations. */

    constexpr void jump() noexcept {
        constexpr uint64_t JUMP[] = {
            0X180EC6D33CFD0ABA,
            0XD5A61266F0C9392C,
            0XA9582618E03FC9AA,
            0X39ABDC4529B1661C
        };
        uint64_t s0 = 0;
        uint64_t s1 = 0;
        uint64_t s2 = 0;
        uint64_t s3 = 0;
        for(int i = 0; i < sizeof(JUMP) / sizeof(*JUMP); i++)
            for(int b = 0; b < 64; b++) {
                if (JUMP[i] & UINT64_C(1) << b) {
                    s0 ^= mState[0];
                    s1 ^= mState[1];
                    s2 ^= mState[2];
                    s3 ^= mState[3];
                }
                next();	
            }
        mState[0] = s0;
        mState[1] = s1;
        mState[2] = s2;
        mState[3] = s3;
    }

    /* This is the long-jump function for the generator. It is equivalent to
    2^192 calls to next(); it can be used to generate 2^64 starting points,
    from each of which jump() will generate 2^64 non-overlapping
    subsequences for parallel distributed computations. */

    constexpr void longJump() noexcept {
        constexpr uint64_t LONG_JUMP[] = {
            0X76E15D3EFEFDCBBF,
            0XC5004E441C522FB3,
            0X77710069854EE241,
            0X39109BB02ACBE635
        };
        uint64_t s0 = 0;
        uint64_t s1 = 0;
        uint64_t s2 = 0;
        uint64_t s3 = 0;
        for(int i = 0; i < sizeof(LONG_JUMP) / sizeof(*LONG_JUMP); i++) {
            for(int b = 0; b < 64; b++) {
                if (LONG_JUMP[i] & UINT64_C(1) << b) {
                    s0 ^= mState[0];
                    s1 ^= mState[1];
                    s2 ^= mState[2];
                    s3 ^= mState[3];
                }
                next();	
            }
        }
        mState[0] = s0;
        mState[1] = s1;
        mState[2] = s2;
        mState[3] = s3;
    }

private:
    std::array<uint64_t, 4> mState;

private:
    constexpr uint64_t next() noexcept {
        const uint64_t result = rotl(mState[1] * 5, 7) * 9;
        const uint64_t t = mState[1] << 17;
        mState[2] ^= mState[0];
        mState[3] ^= mState[1];
        mState[1] ^= mState[2];
        mState[0] ^= mState[3];
        mState[2] ^= t;
        mState[3] = rotl(mState[3], 45);
        return result;
    }

    static constexpr uint64_t rotl(const uint64_t x, int k) noexcept {
        return (x << k) | (x >> (64 - k));
    }

    static constexpr uint64_t splitmix64(uint64_t seed) noexcept {
        uint64_t z = (seed += 0X9E3779B97F4A7C15);
        z = (z ^ (z >> 30)) * 0XBF58476D1CE4E5B9;
        z = (z ^ (z >> 27)) * 0X94D049BB133111EB;
        return z ^ (z >> 31);
    }
};

} // namespace r3d

#endif // R3D_DETAIL_RANDOM_GENERATOR_HPP
