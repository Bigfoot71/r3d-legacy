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

/**
 * @class GLRenderbuffer
 * @brief Represents an OpenGL renderbuffer object, providing storage for render targets such as depth, stencil, or color attachments.
 *
 * This class encapsulates the creation, configuration, and management of OpenGL renderbuffer objects.
 */
class GLRenderbuffer
{
public:
    /**
     * @brief Constructor to create an OpenGL renderbuffer object.
     * 
     * This initializes a new renderbuffer and generates a unique ID.
     */
    GLRenderbuffer();

    /**
     * @brief Destructor to delete the renderbuffer object.
     *
     * Ensures proper cleanup and deletion of the renderbuffer from OpenGL.
     */
    ~GLRenderbuffer();

    /**
     * @brief Deleted copy constructor.
     */
    GLRenderbuffer(const GLRenderbuffer&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    GLRenderbuffer& operator=(const GLRenderbuffer&) = delete;

    /**
     * @brief Move constructor.
     * @param other The GLRenderbuffer instance to move.
     */
    GLRenderbuffer(GLRenderbuffer&& other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The GLRenderbuffer instance to move.
     * @return A reference to the current instance.
     */
    GLRenderbuffer& operator=(GLRenderbuffer&& other) noexcept;

    /**
     * @brief Allocates storage for the renderbuffer.
     * @param format The internal format for the renderbuffer (e.g., GL_DEPTH_COMPONENT).
     * @param width The width of the renderbuffer in pixels.
     * @param height The height of the renderbuffer in pixels.
     * @param samples The number of samples for multisampling (default is 0 for no multisampling).
     */
    void storage(GLenum format, GLsizei width, GLsizei height, GLsizei samples);

    /**
     * @brief Binds the renderbuffer to the current OpenGL context.
     */
    void bind() const;

    /**
     * @brief Unbinds any currently bound renderbuffer.
     */
    static void unbind();

    /**
     * @brief Gets the OpenGL ID of the renderbuffer.
     * @return The renderbuffer ID.
     */
    GLuint id() const { return mID; }

    /**
     * @brief Gets the width of the renderbuffer.
     * @return The width in pixels.
     */
    GLsizei width() const { return mWidth; }

    /**
     * @brief Gets the height of the renderbuffer.
     * @return The height in pixels.
     */
    GLsizei height() const { return mHeight; }

    /**
     * @brief Gets the number of samples for multisampling.
     * @return The number of samples.
     */
    GLsizei samples() const { return mSamples; }

    /**
     * @brief Gets the internal format of the renderbuffer.
     * @return The internal format.
     */
    GLenum format() const { return mFormat; }

    /**
     * @brief Checks if the renderbuffer storage has been allocated.
     * @return True if storage is allocated, false otherwise.
     */
    bool ready() { return mHasStorage; }

private:
    GLuint mID;              ///< OpenGL ID for the renderbuffer.
    GLsizei mWidth;          ///< Width of the renderbuffer.
    GLsizei mHeight;         ///< Height of the renderbuffer.
    GLsizei mSamples;        ///< Number of samples for multisampling.
    GLenum mFormat;          ///< Internal format of the renderbuffer.
    bool mHasStorage;        ///< Indicates whether storage has been allocated.
};


/* Public member functions */

inline GLRenderbuffer::GLRenderbuffer()
    : mHasStorage(false)
{
    glGenRenderbuffers(1, &mID);
}

inline GLRenderbuffer::~GLRenderbuffer()
{
    if (mID > 0) {
        glDeleteRenderbuffers(1, &mID);
    }
}

inline GLRenderbuffer::GLRenderbuffer(GLRenderbuffer&& other) noexcept
    : mID(std::exchange(other.mID, 0))
    , mWidth(std::exchange(other.mWidth, 0))
    , mHeight(std::exchange(other.mHeight, 0))
    , mSamples(std::exchange(other.mSamples, 0))
    , mFormat(std::exchange(other.mFormat, GL_RGBA8))
{ }

inline GLRenderbuffer& GLRenderbuffer::operator=(GLRenderbuffer&& other) noexcept
{
    if (this != &other) {
        mID = std::exchange(other.mID, 0);
        mWidth = std::exchange(other.mWidth, 0);
        mHeight = std::exchange(other.mHeight, 0);
        mSamples = std::exchange(other.mSamples, 0);
        mFormat = std::exchange(other.mFormat, GL_RGBA8);
    }
    return *this;
}

inline void GLRenderbuffer::storage(GLenum format, GLsizei width, GLsizei height, GLsizei samples)
{
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
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    mHasStorage = true;
}

inline void GLRenderbuffer::bind() const
{
    glBindRenderbuffer(GL_RENDERBUFFER, mID);
}

inline void GLRenderbufferunbind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

} // namespace r3d

#endif // R3D_DETAIL_GL_RENDERBUFFER_HPP
