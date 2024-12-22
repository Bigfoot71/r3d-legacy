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

#ifndef R3D_DETAIL_GL_TEXTURE_HPP
#define R3D_DETAIL_GL_TEXTURE_HPP

#include "../build_info.hpp"
#include "../gl.hpp"

#include <raylib.h>

#include <optional>
#include <utility>
#include <cassert>
#include <cmath>

namespace r3d {

/**
 * @class GLTexture
 * @brief Represents an OpenGL texture with utilities for creation and configuration.
 * 
 * This class encapsulates OpenGL texture functionality, providing support for various
 * texture types, wrapping modes, filtering, mipmaps, and resizing.
 */
class GLTexture
{
public:
    /**
     * @enum Wrap
     * @brief Specifies texture wrapping modes.
     */
    enum class Wrap {
        UNKNOWN,       ///< Undefined wrapping mode.
        REPEAT,        ///< Repeats the texture.
        CLAMP_EDGE,    ///< Clamps texture coordinates to the edge.
        CLAMP_BORDER,  ///< Clamps texture coordinates to the border color.
    };

    /**
     * @enum Filter
     * @brief Specifies texture filtering modes.
     */
    enum class Filter {
        UNKNOWN,           ///< Undefined filtering mode.
        NEAREST,           ///< Nearest-neighbor filtering.
        BILINEAR,          ///< Bilinear filtering.
        TRILINEAR,         ///< Trilinear filtering.
        ANISOTROPIC_X4,    ///< 4x anisotropic filtering.
        ANISOTROPIC_X8,    ///< 8x anisotropic filtering.
        ANISOTROPIC_X16,   ///< 16x anisotropic filtering.
    };

public:
    /**
     * @brief Creates a 1D texture.
     * @param data Pointer to the texture data.
     * @param width The width of the texture.
     * @param internalFormat The internal format of the texture.
     * @param format The format of the texture data.
     * @param type The data type of the texture data.
     * @return A GLTexture instance representing the created texture.
     */
    static GLTexture gen1D(const void* data, int width, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * @brief Creates a 2D texture.
     * @param data Pointer to the texture data.
     * @param width The width of the texture.
     * @param height The height of the texture.
     * @param internalFormat The internal format of the texture.
     * @param format The format of the texture data.
     * @param type The data type of the texture data.
     * @return A GLTexture instance representing the created texture.
     */
    static GLTexture gen2D(const void* data, int width, int height, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * @brief Creates a 3D texture.
     * @param data Pointer to the texture data.
     * @param width The width of the texture.
     * @param height The height of the texture.
     * @param depth The depth of the texture.
     * @param internalFormat The internal format of the texture.
     * @param format The format of the texture data.
     * @param type The data type of the texture data.
     * @return A GLTexture instance representing the created texture.
     */
    static GLTexture gen3D(const void* data, int width, int height, int depth, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * @brief Creates a Cube Map texture.
     * @param data Array of pointers to the texture data for each face.
     * @param width The width of each face.
     * @param height The height of each face.
     * @param internalFormat The internal format of the texture.
     * @param format The format of the texture data.
     * @param type The data type of the texture data.
     * @return A GLTexture instance representing the created Cube Map texture.
     */
    static GLTexture genCube(const void** data, int width, int height, GLenum internalFormat, GLenum format, GLenum type);

public:
    /**
     * @brief Default constructor.
     */
    GLTexture() = default;

    /**
     * @brief Destructor to clean up the texture resources.
     */
    ~GLTexture();

    /**
     * @brief Deleted copy constructor.
     */
    GLTexture(const GLTexture&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    GLTexture& operator=(const GLTexture&) = delete;

    /**
     * @brief Move constructor.
     * @param other The GLTexture instance to move.
     */
    GLTexture(GLTexture&& other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The GLTexture instance to move.
     * @return A reference to the current instance.
     */
    GLTexture& operator=(GLTexture&& other) noexcept;

    /**
     * @brief Gets the current wrapping mode of the texture.
     * @return The wrapping mode.
     */
    Wrap wrap() const;

    /**
     * @brief Sets the wrapping mode of the texture.
     * @param wrap_mode The wrapping mode to set.
     */
    void wrap(Wrap wrap_mode);

    /**
     * @brief Gets the current filtering mode of the texture.
     * @return The filtering mode.
     */
    Filter filter() const;

    /**
     * @brief Sets the filtering mode of the texture.
     * @param textureFilter The filtering mode to set.
     */
    void filter(Filter textureFilter);

    /**
     * @brief Gets the border color of the texture, if applicable.
     * @return An optional containing the border color.
     */
    std::optional<Color> borderColor() const;

    /**
     * @brief Sets the border color of the texture.
     * @param color The border color to set.
     */
    void borderColor(Color color);

    /**
     * @brief Gets the number of mipmaps for the texture.
     * @return The number of mipmaps.
     */
    int mipCount() const {
        return mMipmaps;
    }

    /**
     * @brief Generates mipmaps for the texture.
     */
    void genMipmaps();

    /**
     * @brief Resizes the texture.
     * @warning This removes the current texture content.
     * @param newWidth The new width of the texture.
     * @param newHeight The new height of the texture.
     * @param newDepth The new depth of the texture (default is 1).
     */
    void resize(int newWidth, int newHeight, int newDepth = 1);

    /**
     * @brief Gets the OpenGL target of the texture.
     * @return The OpenGL target.
     */
    GLenum target() const {
        return mTarget;
    }

    /**
     * @brief Gets the OpenGL texture ID.
     * @return The texture ID.
     */
    GLuint id() const {
        return mID;
    }

    /**
     * @brief Gets the width of the texture.
     * @return The width in pixels.
     */
    int width() const {
        return mWidth;
    }

    /**
     * @brief Gets the height of the texture.
     * @return The height in pixels.
     */
    int height() const {
        return mHeight;
    }

    /**
     * @brief Gets the internal format of the texture.
     * @return The internal format.
     */
    GLenum internalFormat() const {
        return mInternalFormat;
    }

    /**
     * @brief Gets the format of the texture data.
     * @return The format.
     */
    GLenum format() const {
        return mFormat;
    }

    /**
     * @brief Gets the data type of the texture data.
     * @return The data type.
     */
    GLenum type() const {
        return mType;
    }

    /**
     * @brief Checks if the texture is valid.
     * @return True if the texture is valid, false otherwise.
     */
    bool valid() const {
        return mID > 0;
    }

private:
    GLuint mID{};                           ///< OpenGL texture ID.
    int mWidth{};                           ///< Texture width.
    int mHeight{};                          ///< Texture height.
    int mMipmaps{};                         ///< Number of mipmaps.
    GLuint mTarget{};                       ///< OpenGL target.
    Wrap mWrap{};                           ///< Texture wrapping mode.
    Filter mFilter{};                       ///< Texture filtering mode.
    std::optional<Color> mBorderColor{};    ///< Texture border color, if applicable.
    GLenum mInternalFormat{};               ///< Internal format of the texture.
    GLenum mFormat{};                       ///< Format of the texture data.
    GLenum mType{};                         ///< Data type of the texture data.

private:
    /**
     * @brief Constructs a texture with a specific OpenGL target.
     * @param target The OpenGL target.
     */
    GLTexture(GLenum target);
};


/* Public static functions */

inline GLTexture GLTexture::gen1D(const void* data, int width, GLenum internalFormat, GLenum format, GLenum type)
{
    GLTexture texture(GL_TEXTURE_1D);
    texture.mWidth = width;
    texture.mHeight = 1;
    texture.mMipmaps = 0;
    texture.mInternalFormat = internalFormat;
    texture.mFormat = format;
    texture.mType = type;

    glBindTexture(GL_TEXTURE_1D, texture.mID);

    glTexImage1D(
        GL_TEXTURE_1D, 0, internalFormat,
        width, 0, format, type, data
    );

    if constexpr (Build::DEBUG) {
        glCheckError("GLTexture::gen1D");
    }

    glBindTexture(GL_TEXTURE_1D, 0);

    return texture;
}

inline GLTexture GLTexture::gen2D(const void* data, int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
    GLTexture texture(GL_TEXTURE_2D);
    texture.mWidth = width;
    texture.mHeight = height;
    texture.mMipmaps = 0;
    texture.mInternalFormat = internalFormat;
    texture.mFormat = format;
    texture.mType = type;

    glBindTexture(GL_TEXTURE_2D, texture.mID);

    glTexImage2D(
        GL_TEXTURE_2D, 0, internalFormat,
        width, height, 0, format, type, data
    );

    if constexpr (Build::DEBUG) {
        glCheckError("GLTexture::gen2D");
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

inline GLTexture GLTexture::gen3D(const void* data, int width, int height, int depth, GLenum internalFormat, GLenum format, GLenum type)
{
    GLTexture texture(GL_TEXTURE_3D);
    texture.mWidth = width;
    texture.mHeight = height;
    texture.mMipmaps = 0;
    texture.mInternalFormat = internalFormat;
    texture.mFormat = format;
    texture.mType = type;

    glBindTexture(GL_TEXTURE_3D, texture.mID);

    glTexImage3D(
        GL_TEXTURE_3D, 0, internalFormat,
        width, height, depth, 0, format, type, data
    );

    if constexpr (Build::DEBUG) {
        glCheckError("GLTexture::gen3D");
    }

    glBindTexture(GL_TEXTURE_3D, 0);

    return texture;
}

inline GLTexture GLTexture::genCube(const void** data, int width, int height, GLenum internalFormat, GLenum format, GLenum type)
{
    GLTexture texture(GL_TEXTURE_CUBE_MAP);
    texture.mWidth = width;
    texture.mHeight = height;
    texture.mMipmaps = 0;
    texture.mInternalFormat = internalFormat;
    texture.mFormat = format;
    texture.mType = type;

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture.mID);

    for (int i = 0; i < 6; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F,
            width, height, 0, GL_RED, GL_FLOAT, data
        );
    }

    if constexpr (Build::DEBUG) {
        glCheckError("GLTexture::genCube");
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture;
}


/* Public member functions */

inline GLTexture::~GLTexture()
{
    if (mID != 0) {
        glDeleteTextures(1, &mID);
    }
}

inline GLTexture::GLTexture(GLTexture&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mMipmaps(other.mMipmaps)
    , mTarget(other.mTarget)
    , mWrap(other.mWrap)
    , mFilter(other.mFilter)
    , mBorderColor(other.mBorderColor)
    , mInternalFormat(other.mInternalFormat)
    , mFormat(other.mFormat)
    , mType(other.mType)
{ }

inline GLTexture& GLTexture::operator=(GLTexture&& other) noexcept
{
    if (this != &other) {
        mID = std::exchange(other.mID, 0);
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mMipmaps = other.mMipmaps;
        mTarget = other.mTarget;
        mWrap = other.mWrap;
        mFilter = other.mFilter;
        mBorderColor = other.mBorderColor;
        mInternalFormat = other.mInternalFormat;
        mFormat = other.mFormat;
        mType = other.mType;
    }
    return *this;
}

inline void GLTexture::wrap(Wrap wrap_mode)
{
    mWrap = wrap_mode;
    glBindTexture(mTarget, mID);

    GLenum glWrapMode;
    switch (wrap_mode) {
        case Wrap::REPEAT: glWrapMode = GL_REPEAT; break;
        case Wrap::CLAMP_EDGE: glWrapMode = GL_CLAMP_TO_EDGE; break;
        case Wrap::CLAMP_BORDER: glWrapMode = GL_CLAMP_TO_BORDER; break;
        default: assert(false && "Unknown texture wrap mode"); break;
    }

    glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, glWrapMode);
    glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, glWrapMode);
    if (mTarget == GL_TEXTURE_3D || mTarget == GL_TEXTURE_CUBE_MAP) {
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, glWrapMode);
    }
}

inline void GLTexture::filter(Filter textureFilter)
{
    mFilter = textureFilter;
    glBindTexture(mTarget, mID);

    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;

    switch (textureFilter) {
        case Filter::NEAREST:
            minFilter = GL_NEAREST;
            magFilter = GL_NEAREST;
            break;

        case Filter::BILINEAR:
            minFilter = GL_LINEAR;
            magFilter = GL_LINEAR;
            break;

        case Filter::TRILINEAR:
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
            magFilter = GL_LINEAR;
            break;

        case Filter::ANISOTROPIC_X4:
        case Filter::ANISOTROPIC_X8:
        case Filter::ANISOTROPIC_X16: {
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
            magFilter = GL_LINEAR;

            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);

            GLfloat desiredAniso = 1.0f;
            switch (textureFilter) {
                case Filter::ANISOTROPIC_X4: desiredAniso = 4.0f; break;
                case Filter::ANISOTROPIC_X8: desiredAniso = 8.0f; break;
                case Filter::ANISOTROPIC_X16: desiredAniso = 16.0f; break;
                default: break;
            }

            glTexParameterf(mTarget, GL_TEXTURE_MAX_ANISOTROPY, std::min(desiredAniso, maxAniso));
            break;
        }

        default:
            assert(false && "Unknown texture filter");
            break;
    }

    glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
}

inline void GLTexture::borderColor(Color color)
{
    mBorderColor = color;
    float nCol[4] = {
        static_cast<float>(color.r) / 255,
        static_cast<float>(color.g) / 255,
        static_cast<float>(color.b) / 255,
        static_cast<float>(color.a) / 255
    };
    glBindTexture(mTarget, mID);
    glTexParameterfv(mTarget, GL_TEXTURE_BORDER_COLOR, nCol);
}

inline void GLTexture::genMipmaps()
{
    glBindTexture(mTarget, mID);
    glGenerateMipmap(mTarget);
    mMipmaps = static_cast<int>(std::log2(std::max(mWidth, mHeight))) + 1;
}

inline void GLTexture::resize(int newWidth, int newHeight, int newDepth)
{
    mWidth = newWidth;
    mHeight = newHeight;

    glBindTexture(mTarget, mID);

    switch (mTarget) {
        case GL_TEXTURE_1D:
            glTexImage1D(
                GL_TEXTURE_1D, 0, mInternalFormat,
                mWidth, 0, mFormat, mType, nullptr
            );
            break;

        case GL_TEXTURE_2D:
            glTexImage2D(
                GL_TEXTURE_2D, 0, mInternalFormat,
                mWidth, mHeight, 0, mFormat, mType, nullptr
            );
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(
                GL_TEXTURE_3D, 0, mInternalFormat,
                mWidth, mHeight, newDepth, 0, mFormat, mType, nullptr
            );
            break;

        case GL_TEXTURE_CUBE_MAP:
            for (int i = 0; i < 6; i++) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mInternalFormat,
                    mWidth, mHeight, 0, mFormat, mType, nullptr
                );
            }
            break;

        default:
            if constexpr (Build::DEBUG) {
                assert(false && "Unsupported texture target for resizing");
            }
            break;
    }

    if constexpr (Build::DEBUG) {
        glCheckError("GLTexture::resize");
    }

    if (mBorderColor.has_value()) borderColor(mBorderColor.value());
    if (mFilter != Filter::UNKNOWN) filter(mFilter);
    if (mWrap != Wrap::UNKNOWN) wrap(mWrap);

    glBindTexture(mTarget, 0);
}


/* Private member functions */

inline GLTexture::GLTexture(GLenum target)
    : mTarget(target)
{
    glGenTextures(1, &mID);
    if constexpr (Build::DEBUG) {
        assert(mID != 0 && "Failed to generate texture ID");
    }
}


} // namespace r3d

#endif // R3D_DETAIL_GL_TEXTURE_HPP
