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

#ifndef R3D_DETAIL_RL_TEXTURE_HPP
#define R3D_DETAIL_RL_TEXTURE_HPP

#include <raylib.h>
#include <rlgl.h>

#include <utility>

namespace r3d {

class RLTexture : public ::Texture
{
public:
    RLTexture() = default;

    RLTexture(const ::Texture& texture)
        : ::Texture(texture)
    { }

    // Create a pixel-sized texture of the given color
    // This constructor is used for creating default textures
    RLTexture(::Color color)
        : ::Texture(LoadTextureFromImage({
            static_cast<void*>(&color), 1, 1, 1,
            PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        }))
    { }

    ~RLTexture() {
        if (id > 0) {
            UnloadTexture(*this);
        }
    }

    RLTexture(const RLTexture&) = delete;
    RLTexture& operator=(const RLTexture&) = delete;

    RLTexture(RLTexture&& other) noexcept
        : ::Texture {
            .id = std::exchange(other.id, 0),
            .width = std::exchange(other.width, 0),
            .height = std::exchange(other.height, 0),
            .mipmaps = std::exchange(other.mipmaps, 0),
            .format = std::exchange(other.format, 0)
        }
    { }

    RLTexture& operator=(RLTexture&& other) noexcept {
        if (this != &other) {
            id = std::exchange(other.id, 0);
            width = std::exchange(other.width, 0);
            height = std::exchange(other.height, 0);
            mipmaps = std::exchange(other.mipmaps, 0);
            format = std::exchange(other.format, 0);
        }
        return *this;
    }
};

} // namespace r3d

#endif // R3D_DETAIL_RL_TEXTURE_HPP
