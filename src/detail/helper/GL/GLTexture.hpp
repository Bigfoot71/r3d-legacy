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

#include "../../utils/Build.hpp"
#include "../../GL.hpp"

#include <raylib.h>

#include <optional>
#include <utility>
#include <cassert>
#include <cmath>

namespace r3d {

class GLTexture
{
public:
    enum class Wrap {
        UNKNOWN,
        REPEAT,
        CLAMP_EDGE,
        CLAMP_BORDER,
    };

    enum class Filter {
        UNKNOWN,
        NEAREST,
        BILINEAR,
        TRILINEAR,
        ANISOTROPIC_X4,
        ANISOTROPIC_X8,
        ANISOTROPIC_X16,
    };

public:
    // Fonction statique pour créer une texture 1D
    static GLTexture gen1D(const void* data, int width, GLenum internalFormat, GLenum format, GLenum type) {
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

    // Fonction statique pour créer une texture 2D
    static GLTexture gen2D(const void* data, int width, int height, GLenum internalFormat, GLenum format, GLenum type) {
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

    // Fonction statique pour créer une texture 3D
    static GLTexture gen3D(const void* data, int width, int height, int depth, GLenum internalFormat, GLenum format, GLenum type) {
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

    // Fonction statique pour créer une texture Cube Map
    static GLTexture genCube(const void** data, int width, int height, GLenum internalFormat, GLenum format, GLenum type) {
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

public:
    GLTexture() = default;

    ~GLTexture() {
        if (mID != 0) {
            glDeleteTextures(1, &mID);
        }
    }

    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;

    // Move constructor
    GLTexture(GLTexture&& other) noexcept
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

    // Move assignment operator
    GLTexture& operator=(GLTexture&& other) noexcept {
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

    Wrap wrap() const {
        return mWrap;
    }

    void wrap(Wrap wrap_mode) {
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

    Filter filter() const {
        return mFilter;
    }

    void filter(Filter textureFilter) {
        mFilter = textureFilter;
        glBindTexture(mTarget, mID);

        // Mapper TextureFilter vers OpenGL
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

    std::optional<Color> borderColor() const {
        return mBorderColor;
    }

    void borderColor(Color color) {
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

    int mipLevel() const {
        return mMipmaps;
    }

    void genMipmaps() {
        glBindTexture(mTarget, mID);
        glGenerateMipmap(mTarget);
        mMipmaps = static_cast<int>(std::log2(std::max(mWidth, mHeight))) + 1;
    }

    // WARNING: Removes texture content, as used only for 'RenderTarget'
    void resize(int newWidth, int newHeight, int newDepth = 1) {
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

    GLenum target() const {
        return mTarget;
    }

    GLuint id() const {
        return mID;
    }

    int width() const {
        return mWidth;
    }

    int height() const {
        return mHeight;
    }

    GLenum internalFormat() const {
        return mInternalFormat;
    }

    GLenum format() const {
        return mFormat;
    }

    GLenum type() const {
        return mType;
    }

    bool valid() const {
        return mID > 0;
    }

private:
    GLuint mID{};
    int mWidth{};
    int mHeight{};
    int mMipmaps{};
    GLuint mTarget{};
    Wrap mWrap{};
    Filter mFilter{};
    std::optional<Color> mBorderColor{};
    GLenum mInternalFormat{};
    GLenum mFormat{};
    GLenum mType{};

private:
    GLTexture(GLenum target)
        : mTarget(target)
    {
        glGenTextures(1, &mID);
        if constexpr (Build::DEBUG) {
            assert(mID != 0 && "Failed to generate texture ID");
        }
    }
};

} // namespace r3d

#endif // R3D_DETAIL_GL_TEXTURE_HPP
