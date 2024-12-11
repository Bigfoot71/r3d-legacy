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

#ifndef R3D_DETAIL_BUILD_HPP
#define R3D_DETAIL_BUILD_HPP

namespace r3d {

/**
 * @struct Build
 * @brief Compile-time configuration flags indicating build info.
 * 
 * The `Build` struct provides two static constexpr flags, `RELEASE` 
 * and `DEBUG`, to determine at compile-time whether the code is compiled 
 * in release or debug mode. This enables conditional compilation based on 
 * build type.
 */
struct Build
{
    /**
     * @brief Indicates if the build is a release build.
     * 
     * This constexpr variable evaluates to `true` if `NDEBUG` is defined, which is 
     * typically the case in release builds. Otherwise, it evaluates to `false`, 
     * meaning the build is in debug mode.
     */
    static constexpr bool RELEASE =
    #ifdef NDEBUG
        true;
    #else
        false;
    #endif

    /**
     * @brief Indicates if the build is a debug build.
     * 
     * This constexpr variable is the opposite of `release`. 
     * It evaluates to `true` if `NDEBUG` is not defined (debug mode).
     */
    static constexpr bool DEBUG = !RELEASE;
};

} // namespace r3d

#endif // R3D_DETAIL_BUILD_HPP
