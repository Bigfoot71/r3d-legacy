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

#ifndef R3D_DETAIL_GL_RENDERBUFFER_HPP
#define R3D_DETAIL_GL_RENDERBUFFER_HPP

#include "../../utils/Build.hpp"
#include "../../GL.hpp"
#include <utility>

namespace r3d {

class GLRenderbuffer
{
public:
    GLRenderbuffer(GLenum format, GLsizei width, GLsizei height, GLsizei samples)
        : mFormat(format)
        , mWidth(width)
        , mHeight(height)
        , mSamples(samples)
    {
        glGenRenderbuffers(1, &mID);
        glBindRenderbuffer(GL_RENDERBUFFER, mID);
        if (samples > 0) {
            // Allocate storage for multisampled renderbuffer
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, format, width, height);
        } else {
            // Allocate storage for standard renderbuffer
            glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
        }
        if constexpr (Build::DEBUG) {
            glCheckError("GLRenderbuffer::GLRenderbuffer");
        }
    }

    ~GLRenderbuffer() {
        if (mID > 0) {
            glDeleteRenderbuffers(1, &mID);
        }
    }

    GLRenderbuffer(const GLRenderbuffer&) = delete;
    GLRenderbuffer& operator=(const GLRenderbuffer&) = delete;

    GLRenderbuffer(GLRenderbuffer&& other) noexcept
        : mID(std::exchange(other.mID, 0))
        , mWidth(std::exchange(other.mWidth, 0))
        , mHeight(std::exchange(other.mHeight, 0))
        , mSamples(std::exchange(other.mSamples, 0))
        , mFormat(std::exchange(other.mFormat, GL_RGBA8)) { }

    GLRenderbuffer& operator=(GLRenderbuffer&& other) noexcept {
        if (this != &other) {
            mID = std::exchange(other.mID, 0);
            mWidth = std::exchange(other.mWidth, 0);
            mHeight = std::exchange(other.mHeight, 0);
            mSamples = std::exchange(other.mSamples, 0);
            mFormat = std::exchange(other.mFormat, GL_RGBA8);
        }
        return *this;
    }

    void bind() const {
        glBindRenderbuffer(GL_RENDERBUFFER, mID);
    }

    static void unbind() {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    GLuint id() const { return mID; }
    GLsizei width() const { return mWidth; }
    GLsizei height() const { return mHeight; }
    GLsizei samples() const { return mSamples; }
    GLenum format() const { return mFormat; }

private:
    GLuint mID;
    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mSamples;
    GLenum mFormat;
};

} // namespace r3d

#endif // R3D_DETAIL_GL_RENDERBUFFER_HPP
