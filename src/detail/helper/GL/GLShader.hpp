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

/**
 * @class GLShader
 * @brief Manages an OpenGL shader program, providing utility methods for uniform and texture handling.
 * 
 * This class simplifies the usage of OpenGL shader programs by encapsulating their creation,
 * management, and utility functions for setting values and binding textures.
 *
 * Since this class is for internal use, we don't bother with further verification,
 * the 'setValue' and 'bindTexture' functions assume that the shader in use.
 */
class GLShader
{
public:
    /**
     * @brief Constructs a GLShader object with vertex and fragment shader code.
     * @param vsCode The source code of the vertex shader.
     * @param fsCode The source code of the fragment shader.
     */
    GLShader(const std::string& vsCode, const std::string& fsCode);

    /**
     * @brief Destructor to clean up shader program resources.
     */
    ~GLShader();

    /**
     * @brief Deleted copy constructor.
     */
    GLShader(const GLShader&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    GLShader& operator=(const GLShader&) = delete;

    /**
     * @brief Move constructor.
     * @param other The GLShader instance to move.
     */
    GLShader(GLShader&& other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The GLShader instance to move.
     * @return A reference to the current instance.
     */
    GLShader& operator=(GLShader&& other) noexcept;

    /**
     * @brief Activates the shader program for use.
     */
    void begin() const;

    /**
     * @brief Deactivates the current shader program.
     * 
     * Automatically unbinds textures that were bound during the shader's usage.
     */
    static void end();

    /**
     * @brief Sets the value of a uniform variable in the shader program.
     * @note Must be called between `GLShader::begin()` and `GLShader::end()`.
     * @tparam T The type of the value to set.
     * @param name The name of the uniform variable.
     * @param value The value to set.
     */
    template <typename T>
    void setValue(const std::string& name, const T& value) const;

    /**
     * @brief Sets a color value for a uniform variable in the shader program.
     * @note Must be called between `GLShader::begin()` and `GLShader::end()`.
     * @param name The name of the uniform variable.
     * @param color The color value to set.
     * @param alpha Whether to include the alpha component.
     */
    void setColor(const std::string& name, ::Color color, bool alpha) const;

    /**
     * @brief Binds a texture to the shader program.
     * @note Must be called between `GLShader::begin()` and `GLShader::end()`.
     * @param name The name of the uniform sampler variable.
     * @param target The texture target (e.g., GL_TEXTURE_2D).
     * @param id The OpenGL texture ID.
     */
    void bindTexture(const std::string& name, GLenum target, GLuint id) const;

    /**
     * @brief Binds a GLTexture object to the shader program.
     * @note Must be called between `GLShader::begin()` and `GLShader::end()`.
     * @param name The name of the uniform sampler variable.
     * @param texture The GLTexture object to bind.
     */
    void bindTexture(const std::string& name, const GLTexture& texture) const;

    /**
     * @brief Unbinds all textures accumulated during shader usage.
     * 
     * This function is useful when re-sending new textures to the same shader without calling `GLShader::end()`.
     */
    static void unbindTextures();

private:
    /**
     * @brief Initializes a sampler uniform variable in the shader program.
     * @param type The type of the sampler.
     * @param location The location of the sampler uniform.
     */
    void initSampler(GLenum type, GLint location) const;

private:
    std::unordered_map<std::string, GLint> mUniforms;       ///< Map of uniform names to locations.
    GLuint mID;                                             ///< OpenGL shader program ID.

private:
    static inline std::array<GLenum, 32> sTextureTypes{};   ///< Array of texture types for binding.
    static inline int sBindCount{};                         ///< Count of currently bound textures.
};


/* Public member functions */

inline GLShader::GLShader(const std::string& vsCode, const std::string& fsCode)
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

inline GLShader::~GLShader()
{
    if (mID > 0) {
        rlUnloadShaderProgram(mID);
    }
}

inline GLShader::GLShader(GLShader&& other) noexcept
    : mUniforms(std::move(other.mUniforms))
    , mID(std::exchange(other.mID, 0))
{ }

inline GLShader& GLShader::operator=(GLShader&& other) noexcept {
    if (this != &other) {
        mUniforms = std::move(other.mUniforms);
        mID = std::exchange(other.mID, 0);
    }
    return *this;
}

inline void GLShader::begin() const
{
    glUseProgram(mID);
}

inline void GLShader::end()
{
    glUseProgram(0);
    unbindTextures();
}

template <typename T>
inline void GLShader::setValue(const std::string& name, const T& value) const
{
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

inline void GLShader::setColor(const std::string& name, ::Color color, bool alpha) const
{
    GLint loc = mUniforms.at(name);
    if (alpha) {
        glUniform4f(loc, color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
    } else {
        glUniform3f(loc, color.r / 255.f, color.g / 255.f, color.b / 255.f);
    }
}

inline void GLShader::bindTexture(const std::string& name, GLenum target, GLuint id) const
{
    GLint loc = mUniforms.at(name);
    glActiveTexture(GL_TEXTURE0 + sBindCount);
    glBindTexture(target, id);
    glUniform1i(loc, sBindCount);
    sTextureTypes[sBindCount] = target;
    sBindCount++;
}

inline void GLShader::bindTexture(const std::string& name, const GLTexture& texture) const
{
    bindTexture(name, texture.target(), texture.id());
}

inline void GLShader::unbindTextures()
{
    for (int i = sBindCount - 1; i > 0; i--) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(sTextureTypes[i], 0);
    }
    sBindCount = 0;
}


/* Private member functions */

inline void GLShader::initSampler(GLenum type, GLint location) const
{
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

} // namespace r3d

#endif // R3D_DETAIL_GL_SHADER_HPP
