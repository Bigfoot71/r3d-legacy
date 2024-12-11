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

#ifndef R3D_DETAIL_RL_SHADER_HPP
#define R3D_DETAIL_RL_SHADER_HPP

#include <raylib.h>
#include <rlgl.h>

#include <utility>
#include <string>

namespace r3d {

class RLShader : public ::Shader
{
public:
    RLShader()
        : ::Shader{}
    { }

    RLShader(const ::Shader& shader)
        : ::Shader(shader)
    { }

    RLShader(const char* vsCode, const char* fsCode)
        : ::Shader(LoadShaderFromMemory(vsCode, fsCode))
    { }

    ~RLShader() {
        if (id > 0) {
            UnloadShader(*this);
        }
    }

    RLShader(const RLShader&) = delete;
    RLShader& operator=(const RLShader&) = delete;

    RLShader(RLShader&& other) noexcept
        : ::Shader {
            .id = std::exchange(other.id, 0),
            .locs = std::exchange(other.locs, nullptr)
        }
    { }

    RLShader& operator=(RLShader&& other) noexcept {
        if (this != &other) {
            id = std::exchange(other.id, 0);
            locs = std::exchange(other.locs, nullptr);
        }
        return *this;
    }

    int location(const std::string& uniformName) const {
        return GetShaderLocation(*this, uniformName.c_str());
    }

    void begin() const {
        BeginShaderMode(*this);
    }

    void end() const {
        EndShaderMode();
    }

    void use() const {
        rlEnableShader(id);
    }

    bool valid() const {
        return id > 0;
    }
};

} // namespace r3d

#endif // R3D_DETAIL_RL_SHADER_HPP
