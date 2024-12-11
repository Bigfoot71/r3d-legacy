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

#ifndef R3D_DETAIL_GL_SHADER_HPP
#define R3D_DETAIL_GL_SHADER_HPP

#include "../../GL.hpp"

#include "./GLTexture.hpp"

#include <raylib.h>
#include <rlgl.h>

#include <unordered_map>
#include <type_traits>
#include <utility>
#include <string>
#include <array>

// TODO: This class is very convenient but not the most efficient.
//       It would be a good idea to write specific classes for certain critical shaders,
//       such as the one for materials.

namespace r3d {

class GLShader
{
public:
    GLShader(const std::string& vsCode, const std::string& fsCode)
        : mID(rlLoadShaderCode(vsCode.c_str(), fsCode.c_str()))
    {
        assert(mID != 0);

        glUseProgram(mID);  ///< We use the program to initialize the samplers

        GLint numUniforms = 0;
        glGetProgramiv(mID, GL_ACTIVE_UNIFORMS, &numUniforms);

        for (GLint i = 0; i < numUniforms; i++) {
            char name[64]{};
            GLsizei length = 0;
            GLenum type = 0;
            GLint size = 0;

            glGetActiveUniform(mID, i, sizeof(name), &length, &size, &type, name);
            GLint location = glGetUniformLocation(mID, name);
            mUniforms.emplace(name, location);
            initSampler(type, location);

            // NOTE: The function 'glGetProgramiv' with 'GL_ACTIVE_UNIFORMS' returns the number of uniforms,
            //       but it only counts the first element of uniform arrays ('u[0]'). Therefore, we ensure
            //       that we retrieve all of them for caching purposes.

            std::string strName(name);

            if (strName.ends_with("[0]")) {
                for (int i = 1;; i++) {
                    strName.replace(strName.size() - 2, 1, std::to_string(i));
                    GLint location = glGetUniformLocation(mID, strName.c_str());
                    if (location == -1) break;
                    mUniforms.emplace(strName, location);
                    initSampler(type, location);
                }
            }
        }

        glUseProgram(0);
    }

    ~GLShader() {
        if (mID > 0) {
            rlUnloadShaderProgram(mID);
        }
    }

    GLShader(const GLShader&) = delete;
    GLShader& operator=(const GLShader&) = delete;

    GLShader(GLShader&& other) noexcept
        : mUniforms(std::move(other.mUniforms))
        , mID(std::exchange(other.mID, 0))
    { }

    GLShader& operator=(GLShader&& other) noexcept {
        if (this != &other) {
            mUniforms = std::move(other.mUniforms);
            mID = std::exchange(other.mID, 0);
        }
        return *this;
    }

    void begin() const {
        glUseProgram(mID);
    }

    // NOTE: Unbind automatiquement les textures qui l'ont été pendant l'utilisation du shader
    static void end() {
        glUseProgram(0);
        unbindTextures();
    }

    // Since this class is for internal use, we don't bother with further verification,
    // the 'setValue' and 'bindTexture' functions assume that the shader in use.

    // Doit etre appeler entre "GLShader::begin()" et "GLShader::end()"
    template <typename T>
    void setValue(const std::string& name, const T& value) const {
        GLint loc = mUniforms.at(name);
        if constexpr (std::is_same_v<T, bool>) {
            int v = value; glUniform1i(loc, v);
        } else if constexpr (std::is_same_v<T, int>) {
            glUniform1i(loc, value);
        } else if constexpr (std::is_same_v<T, float>) {
            glUniform1f(loc, value);
        } else if constexpr (std::is_same_v<T, double>) {
            glUniform1f(loc, static_cast<float>(value));
        } else if constexpr (std::is_same_v<T, ::Vector2>) {
            glUniform2f(loc, value.x, value.y);
        } else if constexpr (std::is_same_v<T, ::Vector3>) {
            glUniform3f(loc, value.x, value.y, value.z);
        } else if constexpr (std::is_same_v<T, ::Vector4>) {
            glUniform4f(loc, value.x, value.y, value.z, value.w);
        } else if constexpr (std::is_same_v<T, ::Matrix>) {
            rlSetUniformMatrix(loc, value);
        }
    }

    // Doit etre appeler entre "GLShader::begin()" et "GLShader::end()"
    void setColor(const std::string& name, ::Color color, bool alpha) const {
        GLint loc = mUniforms.at(name);
        if (alpha) {
            glUniform4f(loc, color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        } else {
            glUniform3f(loc, color.r / 255.f, color.g / 255.f, color.b / 255.f);
        }
    }

    // Doit etre appeler entre "GLShader::begin()" et "GLShader::end()"
    void bindTexture(const std::string& name, GLenum target, GLuint id) const {
        GLint loc = mUniforms.at(name);
        glActiveTexture(GL_TEXTURE0 + sBindCount);
        glBindTexture(target, id);
        glUniform1i(loc, sBindCount);
        sTextureTypes[sBindCount] = target;
        sBindCount++;
    }

    // Doit etre appeler entre "GLShader::begin()" et "GLShader::end()"
    void bindTexture(const std::string& name, const GLTexture& texture) const {
        bindTexture(name, texture.target(), texture.id());
    }

    // NOTE: Les bind sont toujours accumulé, même si vous bindez deux fois la meme texture
    // Cette fonction est donc utile si vous n'appelez pas "GLShader::end()" mais que vous souhaitez
    // re-envoyer de nouvelles textures aux mêmes uniforms du meme shader qui est resté actif
    static void unbindTextures() {
        for (int i = sBindCount - 1; i > 0; i--) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(sTextureTypes[i], 0);
        }
        sBindCount = 0;
    }

private:
    void initSampler(GLenum type, GLint location) const {
        // Here we initialize the samplers with different values
        // because if samplers of different types are assigned to the same texture unit,
        // the driver may refuse to render, and the default value for samplers (uniforms) is '0'.
        // All samplers, even of different types, are theoretically assigned to slot '0' during initialization,
        // which can lead to no rendering.

        // WARNING: This method may not work for all configurations.
        // Depending on what we do with the samplers and texture units afterwards, if a black screen
        // occurs after updating shaders or CPU-side logic, modifying the default values may help diagnose
        // if the problem is coming from there. I tried to create the method with minimal code, and it works perfectly so far.

        switch (type) {
            case GL_SAMPLER_2D:
                // Here we keep to 0 by default
                break;
            case GL_SAMPLER_1D:
                glUniform1i(location, 1);
                break;
            case GL_SAMPLER_3D:
                glUniform1i(location, 2);
                break;
            case GL_SAMPLER_CUBE:
                glUniform1i(location, 3);
                break;
            default:
                // There are of course other types, but not used in R3D.
                break;
        }
    }

private:
    std::unordered_map<std::string, GLint> mUniforms;
    GLuint mID;

private:
    static inline std::array<GLenum, 32> sTextureTypes{};
    static inline int sBindCount{};
};

} // namespace r3d

#endif // R3D_DETAIL_GL_SHADER_HPP
