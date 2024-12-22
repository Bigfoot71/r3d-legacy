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

#include "./gl_texture.hpp"

#include "../build_info.hpp"
#include "../gl.hpp"

#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace r3d {

/**
 * @enum GLAttachement
 * @brief Enumerates OpenGL attachment points for framebuffers.
 *
 * Provides a set of constants representing attachment points for framebuffer objects, 
 * including color, depth, and stencil attachments.
 */
enum class GLAttachement : GLenum
{
    NONE            = GL_NONE,                      ///< No attachment.
    COLOR_0         = GL_COLOR_ATTACHMENT0,         ///< First color attachment.
    COLOR_1         = GL_COLOR_ATTACHMENT1,         ///< Second color attachment.
    COLOR_2         = GL_COLOR_ATTACHMENT2,         ///< Third color attachment.
    COLOR_3         = GL_COLOR_ATTACHMENT3,         ///< Fourth color attachment.
    COLOR_4         = GL_COLOR_ATTACHMENT4,         ///< Fifth color attachment.
    COLOR_5         = GL_COLOR_ATTACHMENT5,         ///< Sixth color attachment.
    COLOR_6         = GL_COLOR_ATTACHMENT6,         ///< Seventh color attachment.
    COLOR_7         = GL_COLOR_ATTACHMENT7,         ///< Eighth color attachment.
    COLOR_8         = GL_COLOR_ATTACHMENT8,         ///< Ninth color attachment.
    COLOR_9         = GL_COLOR_ATTACHMENT9,         ///< Tenth color attachment.
    STENCIL         = GL_STENCIL_ATTACHMENT,        ///< Stencil attachment.
    DEPTH_STENCIL   = GL_DEPTH_STENCIL_ATTACHMENT,  ///< Depth and stencil attachment.
    DEPTH           = GL_DEPTH_ATTACHMENT           ///< Depth attachment.
};

/**
 * @class GLFramebuffer
 * @brief Encapsulates an OpenGL framebuffer object.
 *
 * Manages the creation, configuration, and binding of framebuffers, allowing
 * the attachment of textures and renderbuffers.
 */
class GLFramebuffer
{
public:
    /**
     * @brief Constructs a new framebuffer object.
     */
    GLFramebuffer();

    /**
     * @brief Destroys the framebuffer object.
     *
     * Ensures proper cleanup of OpenGL resources.
     */
    ~GLFramebuffer();

    /**
     * @brief Deleted copy constructor.
     */
    GLFramebuffer(const GLFramebuffer&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    GLFramebuffer& operator=(const GLFramebuffer&) = delete;

    /**
     * @brief Move constructor.
     * @param other The framebuffer to move from.
     */
    GLFramebuffer(GLFramebuffer&& other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The framebuffer to move from.
     * @return A reference to the current framebuffer.
     */
    GLFramebuffer& operator=(GLFramebuffer&& other) noexcept;

    /**
     * @brief Attaches a texture to the framebuffer.
     * @param attach The attachment point.
     * @param texture The texture to attach.
     * @param mipLevel The mipmap level to attach (default is 0).
     */
    void attachTexture(GLAttachement attach, const GLTexture& texture, int mipLevel = 0) const;

    /**
     * @brief Attaches a renderbuffer to the framebuffer.
     * @param attach The attachment point.
     * @param renderbufferID The renderbuffer's ID.
     */
    void attachRenderbuffer(GLAttachement attach, GLuint renderbufferID) const;

    /**
     * @brief Binds the framebuffer for use.
     */
    void bind() const;

    /**
     * @brief Unbinds the currently bound framebuffer.
     */
    static void unbind();

    /**
     * @brief Gets the OpenGL ID of the framebuffer.
     * @return The framebuffer ID.
     */
    GLuint id() const { return mID; }

    /**
     * @brief Configures the draw buffer for the framebuffer.
     * @param buffer The attachment point for drawing.
     */
    void setDrawBuffer(GLAttachement buffer) const;

    /**
     * @brief Configures multiple draw buffers for the framebuffer.
     * @param buffers A list of attachment points for drawing.
     */
    void setDrawBuffers(const std::initializer_list<GLAttachement>& buffers) const;

    /**
     * @brief Configures the read buffer for the framebuffer.
     * @param buffer The attachment point for reading.
     */
    void setReadBuffer(GLAttachement buffer) const;

    /**
     * @brief Checks if the framebuffer is complete.
     * @param target The framebuffer target (default is GL_FRAMEBUFFER).
     * @return True if the framebuffer is complete, false otherwise.
     */
    static bool isComplete(GLenum target = GL_FRAMEBUFFER);

    /**
     * @brief Checks the status of the framebuffer.
     * @param target The framebuffer target (default is GL_FRAMEBUFFER).
     *
     * If the framebuffer is incomplete, throws an exception or logs an error.
     */
    static void checkStatus(GLenum target = GL_FRAMEBUFFER);

private:
    /**
     * @brief Converts a framebuffer status code to a human-readable string.
     * @param status The framebuffer status code.
     * @return A string describing the status.
     */
    static std::string statusToString(GLenum status);

private:
    GLuint mID = 0; ///< OpenGL ID of the framebuffer.
};


/* Public member functions */

inline GLFramebuffer::GLFramebuffer()
{
    glGenFramebuffers(1, &mID);
}

inline GLFramebuffer::~GLFramebuffer()
{
    if (mID > 0) {
        glDeleteFramebuffers(1, &mID);
    }
}

inline GLFramebuffer::GLFramebuffer(GLFramebuffer&& other) noexcept
    : mID(std::exchange(other.mID, 0))
{ }

inline GLFramebuffer& GLFramebuffer::operator=(GLFramebuffer&& other) noexcept
{
    if (this != &other) {
        mID = std::exchange(other.mID, 0);
    }
    return *this;
}

inline void GLFramebuffer::attachTexture(GLAttachement attach, const GLTexture& texture, int mipLevel) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mID);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER, static_cast<GLenum>(attach),
            texture.target(), texture.id(), mipLevel
        );

        if constexpr (Build::DEBUG) {
            glCheckError("GLFramebuffer::attachTexture");
        }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void GLFramebuffer::attachRenderbuffer(GLAttachement attach, GLuint renderbufferID) const
{
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

inline void GLFramebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mID);
}

inline void GLFramebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void GLFramebuffer::setDrawBuffer(GLAttachement buffer) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mID);
    glDrawBuffer(static_cast<GLenum>(buffer));
    if constexpr (Build::DEBUG) {
        glCheckError("GLFramebuffer::setDrawBuffer");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void GLFramebuffer::setDrawBuffers(const std::initializer_list<GLAttachement>& buffers) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mID);
    glDrawBuffers(static_cast<GLsizei>(buffers.size()), reinterpret_cast<const GLenum*>(buffers.begin()));

    if constexpr (Build::DEBUG) {
        glCheckError("GLFramebuffer::setDrawBuffers");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void GLFramebuffer::setReadBuffer(GLAttachement buffer) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mID);
    glReadBuffer(static_cast<GLenum>(buffer));

    if constexpr (Build::DEBUG) {
        glCheckError("GLFramebuffer::setReadBuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


/* Public static functions */

inline bool GLFramebuffer::isComplete(GLenum target)
{
    return glCheckFramebufferStatus(target) == GL_FRAMEBUFFER_COMPLETE;
}

inline void GLFramebuffer::checkStatus(GLenum target)
{
    GLenum status = glCheckFramebufferStatus(target);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("GLFramebuffer is not complete: " + statusToString(status));
    }
}


/* Private static functions */

inline std::string GLFramebuffer::statusToString(GLenum status)
{
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

} // namespace r3d

#endif // R3D_DETAIL_GL_FRAMEBUFFER_HPP
