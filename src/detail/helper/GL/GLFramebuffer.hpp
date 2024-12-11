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

#ifndef R3D_DETAIL_GL_FRAMEBUFFER_HPP
#define R3D_DETAIL_GL_FRAMEBUFFER_HPP

#include "./GLTexture.hpp"

#include "../../utils/Build.hpp"
#include "../../GL.hpp"

#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace r3d {

enum class GLAttachement : GLenum
{
    NONE            = GL_NONE,
    COLOR_0         = GL_COLOR_ATTACHMENT0,
    COLOR_1         = GL_COLOR_ATTACHMENT1,
    COLOR_2         = GL_COLOR_ATTACHMENT2,
    COLOR_3         = GL_COLOR_ATTACHMENT3,
    COLOR_4         = GL_COLOR_ATTACHMENT4,
    COLOR_5         = GL_COLOR_ATTACHMENT5,
    COLOR_6         = GL_COLOR_ATTACHMENT6,
    COLOR_7         = GL_COLOR_ATTACHMENT7,
    COLOR_8         = GL_COLOR_ATTACHMENT8,
    COLOR_9         = GL_COLOR_ATTACHMENT9,
    STENCIL         = GL_STENCIL_ATTACHMENT,
    DEPTH_STENCIL   = GL_DEPTH_STENCIL_ATTACHMENT,
    DEPTH           = GL_DEPTH_ATTACHMENT
};

class GLFramebuffer
{
public:
    GLFramebuffer() {
        glGenFramebuffers(1, &mID);
    }

    ~GLFramebuffer() {
        if (mID > 0) {
            glDeleteFramebuffers(1, &mID);
        }
    }

    GLFramebuffer(const GLFramebuffer&) = delete;
    GLFramebuffer& operator=(const GLFramebuffer&) = delete;

    GLFramebuffer(GLFramebuffer&& other) noexcept
        : mID(std::exchange(other.mID, 0))
    { }

    GLFramebuffer& operator=(GLFramebuffer&& other) noexcept {
        if (this != &other) {
            mID = std::exchange(other.mID, 0);
        }
        return *this;
    }

    void attachTexture(GLAttachement attach, const GLTexture& texture) const {
        glBindFramebuffer(GL_FRAMEBUFFER, mID);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER, static_cast<GLenum>(attach),
                texture.target(), texture.id(), texture.mipLevel()
            );

            if constexpr (Build::DEBUG) {
                glCheckError("GLFramebuffer::attachTexture");
            }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void attachRenderbuffer(GLAttachement attach, GLuint renderbufferID) const {
        glBindFramebuffer(GL_FRAMEBUFFER, mID);

            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, static_cast<GLenum>(attach),
                GL_RENDERBUFFER, renderbufferID
            );

            if constexpr (Build::DEBUG) {
                glCheckError("GLFramebuffer::attachRenderbuffer");
            }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, mID);
    }

    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint id() const { return mID; }

    // Configure the draw buffer for the framebuffer
    void setDrawBuffer(GLAttachement buffer) const {
        glBindFramebuffer(GL_FRAMEBUFFER, mID);
        glDrawBuffer(static_cast<GLenum>(buffer));
        if constexpr (Build::DEBUG) {
            glCheckError("GLFramebuffer::setDrawBuffer");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Configure multiple draw buffers for the framebuffer
    void setDrawBuffers(const std::initializer_list<GLAttachement>& buffers) const {
        glBindFramebuffer(GL_FRAMEBUFFER, mID);
        glDrawBuffers(static_cast<GLsizei>(buffers.size()), reinterpret_cast<const GLenum*>(buffers.begin()));
        if constexpr (Build::DEBUG) {
            glCheckError("GLFramebuffer::setDrawBuffers");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Configure the read buffer for the framebuffer
    void setReadBuffer(GLAttachement buffer) const {
        glBindFramebuffer(GL_FRAMEBUFFER, mID);
        glReadBuffer(static_cast<GLenum>(buffer));
        if constexpr (Build::DEBUG) {
            glCheckError("GLFramebuffer::setReadBuffer");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Check if the framebuffer is complete
    static bool isComplete(GLenum target = GL_FRAMEBUFFER) {
        return glCheckFramebufferStatus(target) == GL_FRAMEBUFFER_COMPLETE;
    }

    // Check the status of the framebuffer
    static void checkStatus(GLenum target = GL_FRAMEBUFFER) {
        GLenum status = glCheckFramebufferStatus(target);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("GLFramebuffer is not complete: " + statusToString(status));
        }
    }

private:
    static std::string statusToString(GLenum status) {
        switch (status) {
            case GL_FRAMEBUFFER_COMPLETE:
                return "Framebuffer is complete";
            case GL_FRAMEBUFFER_UNDEFINED:
                return "Framebuffer is undefined";
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                return "Framebuffer has incomplete attachment";
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                return "Framebuffer has missing attachment";
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                return "Framebuffer has incomplete draw buffer";
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                return "Framebuffer has incomplete read buffer";
            case GL_FRAMEBUFFER_UNSUPPORTED:
                return "Framebuffer format is unsupported";
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                return "Framebuffer has incomplete multisample configuration";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                return "Framebuffer has incomplete layer targets";
            default:
                return "Unknown framebuffer status";
        }
    }

private:
    GLuint mID = 0;
};

} // namespace r3d

#endif // R3D_DETAIL_GL_FRAMEBUFFER_HPP
