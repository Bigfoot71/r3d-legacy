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

#ifndef R3D_DETAIL_RENDER_TARGET_HPP
#define R3D_DETAIL_RENDER_TARGET_HPP

#include "./GL.hpp"

#include "./GL/GLTexture.hpp"
#include "./GL/GLFramebuffer.hpp"

#include <raylib.h>
#include <rlgl.h>

#include <initializer_list>
#include <cassert>
#include <map>

namespace r3d {

/**
 * @class RenderTarget
 * @brief A generic container for rendering targets such as shadow maps, post-processing targets, and more.
 * 
 * This class provides utilities for managing and configuring OpenGL rendering targets, including
 * attachment creation, buffer configuration, resizing, and advanced blitting operations.
 */
class RenderTarget
{
public:
    /**
     * @brief Constructor to initialize the render target with a given width and height.
     * @param w The width of the render target.
     * @param h The height of the render target.
     */
    RenderTarget(int w, int h);

    /**
     * @brief Creates an attachment for the render target.
     * @param attach The attachment type (e.g., color, depth).
     * @param target The OpenGL target (e.g., GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP).
     * @param internalFormat The internal format of the texture (e.g., GL_RGBA).
     * @param format The format of the texture data (e.g., GL_RGBA).
     * @param type The data type of the texture data (e.g., GL_UNSIGNED_BYTE).
     * @return A reference to the created texture.
     */
    GLTexture& createAttachment(GLAttachement attach, GLenum target, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * @brief Sets the draw buffers for the render target.
     * @param list A list of attachments to be used as draw buffers.
     */
    void setDrawBuffers(const std::initializer_list<GLAttachement>& list) const;

    /**
     * @brief Sets a single draw buffer for the render target.
     * @param attach The attachment to be used as the draw buffer.
     */
    void setDrawBuffer(GLAttachement attach) const;

    /**
     * @brief Sets the read buffer for the render target.
     * @param attach The attachment to be used as the read buffer.
     */
    void setReadBuffer(GLAttachement attach) const;

    /**
     * @brief Binds the render target for rendering operations.
     */
    void begin() const;

    /**
     * @brief Ends rendering operations on the current render target.
     */
    static void end();

    /**
     * @brief Binds a specific face of a cube map attachment for rendering.
     * @param attach The attachment to bind.
     * @param face The face index of the cube map (0 to 5).
     */
    void bindFace(GLAttachement attach, int face) const;

    /**
     * @brief Retrieves a non-const reference to an attachment texture.
     * @param attach The attachment to retrieve.
     * @return A reference to the attachment texture.
     */
    GLTexture& attachement(GLAttachement attach);

    /**
     * @brief Retrieves a const reference to an attachment texture.
     * @param attach The attachment to retrieve.
     * @return A const reference to the attachment texture.
     */
    const GLTexture& attachement(GLAttachement attach) const;

    /**
     * @brief Resizes the attachments of the render target.
     * @param newWidth The new width of the render target.
     * @param newHeight The new height of the render target.
     */
    void resize(int newWidth, int newHeight);

    /**
     * @brief Draws a specific attachment onto a specified region.
     * @param attach The attachment to draw.
     * @param x The x-coordinate of the target region.
     * @param y The y-coordinate of the target region.
     * @param w The width of the target region.
     * @param h The height of the target region.
     */
    void draw(GLAttachement attach, int x, int y, int w, int h) const;

    /**
     * @brief Blits the entire buffer to another framebuffer, expanding to fit the window aspect ratio.
     * @param fbTarget The target framebuffer.
     * @param attach The attachment to blit (default is COLOR_0).
     * @param blitDepth Whether to include the depth buffer in the blit.
     * @param linear Whether to use linear filtering during the blit.
     */
    void blitAspectExpand(GLuint fbTarget, GLAttachement attach = GLAttachement::COLOR_0, bool blitDepth = true, bool linear = false) const;

    /**
     * @brief Blits the entire buffer to another framebuffer, keeping the internal resolution aspect ratio.
     * @param fbTarget The target framebuffer.
     * @param attach The attachment to blit (default is COLOR_0).
     * @param blitDepth Whether to include the depth buffer in the blit.
     * @param linear Whether to use linear filtering during the blit.
     */
    void blitAspectKeep(GLuint fbTarget, GLAttachement attach = GLAttachement::COLOR_0, bool blitDepth = true, bool linear = false) const;

    /**
     * @brief Retrieves the width of the render target.
     * @return The width in pixels.
     */
    int width() const {
        return mWidth;
    }

    /**
     * @brief Retrieves the height of the render target.
     * @return The height in pixels.
     */
    int height() const {
        return mHeight;
    }

    /**
     * @brief Retrieves the texel width of the render target.
     * @return The texel width as a float.
     */
    int texelWidth() const {
        return mTxlW;
    }

    /**
     * @brief Retrieves the texel height of the render target.
     * @return The texel height as a float.
     */
    int texelHeight() const {
        return mTxlH;
    }

private:
    /**
     * @brief Creates a texture for use as an attachment.
     * @note Only supports GL_TEXTURE_2D and GL_TEXTURE_CUBE_MAP.
     * @param target The OpenGL target (e.g., GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP).
     * @param internalFormat The internal format of the texture.
     * @param format The format of the texture data.
     * @param type The data type of the texture data.
     * @return The created texture.
     */
    GLTexture createTexture(GLenum target, GLenum internalFormat, GLenum format, GLenum type);

    /**
     * @brief Converts a GLAttachement enum to an OpenGL attachment type.
     * @param attach The GLAttachement to convert.
     * @return The corresponding OpenGL attachment type.
     */
    static GLenum glAttachement(GLAttachement attach);

private:
    std::map<GLAttachement, GLTexture> mAttachments;    ///< Map of attachments for the render target.
    GLFramebuffer mFramebuffer;                         ///< The framebuffer associated with the render target.
    int mWidth;                                         ///< The width of the render target.
    int mHeight;                                        ///< The height of the render target.
    float mTxlW;                                        ///< The texel width.
    float mTxlH;                                        ///< The texel height.
};

/* Public implementation */

inline RenderTarget::RenderTarget(int w, int h)
    : mWidth(w), mHeight(h)
    , mTxlW(1.0f / w)
    , mTxlH(1.0f / h)
{ }

inline GLTexture& RenderTarget::createAttachment(GLAttachement attach, GLenum target, GLenum internalFormat, GLenum format, GLenum type)
{
    assert(attach != GLAttachement::NONE && "The type of attachment given during creation cannot be 'NONE'");

    auto texture = createTexture(target, internalFormat, format, type);

    if (target != GL_TEXTURE_CUBE_MAP) {
        mFramebuffer.bind();
            mFramebuffer.attachTexture(attach, texture);
            if constexpr (Build::DEBUG) {
                mFramebuffer.checkStatus();
            }
        mFramebuffer.unbind();
    }

    return (mAttachments[attach] = std::move(texture));
}

inline void RenderTarget::setDrawBuffers(const std::initializer_list<GLAttachement>& list) const
{
    mFramebuffer.setDrawBuffers(list);
}

inline void RenderTarget::setDrawBuffer(GLAttachement attach) const
{
    mFramebuffer.setDrawBuffer(attach);
}

inline void RenderTarget::setReadBuffer(GLAttachement attach) const
{
    mFramebuffer.setReadBuffer(attach);
}

inline void RenderTarget::begin() const
{
    mFramebuffer.bind();
    glViewport(0, 0, mWidth, mHeight);
}

inline void RenderTarget::end()
{
    GLFramebuffer::unbind();
}

inline void RenderTarget::bindFace(GLAttachement attach, int face) const
{
    const auto& texture = mAttachments.at(attach);
    if (texture.target() == GL_TEXTURE_CUBE_MAP) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture.id(), 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    } else {
        assert(false && "'RenderTarget::setFace()' should be called on cubemaps only");
    }
}

inline GLTexture& RenderTarget::attachement(GLAttachement attach)
{
    return mAttachments.at(attach);
}

inline const GLTexture& RenderTarget::attachement(GLAttachement attach) const
{
    return mAttachments.at(attach);
}

inline void RenderTarget::resize(int newWidth, int newHeight)
{
    mWidth = newWidth, mHeight = newHeight;
    mTxlW = 1.0f / newWidth, mTxlH = 1.0f / newHeight;
    for (auto& [_, texture] : mAttachments) {
        if (texture.valid()) {
            texture.resize(newWidth, newHeight);
        }
    }
}

inline void RenderTarget::draw(GLAttachement attach, int x, int y, int w, int h) const
{
    rlSetTexture(mAttachments.at(attach).id());
    rlBegin(RL_QUADS);

        rlColor4ub(255, 255, 255, 255);
        rlNormal3f(0.0f, 0.0f, 1.0f);

        // Bottom-left corner for texture and quad
        rlTexCoord2f(0.0f, 0.0f);
        rlVertex2f(x, y + h);

        // Bottom-right corner for texture and quad
        rlTexCoord2f(1.0f, 0.0f);
        rlVertex2f(x + w, y + h);

        // Top-right corner for texture and quad
        rlTexCoord2f(1.0f, 1.0f);
        rlVertex2f(x + w, y);

        // Top-left corner for texture and quad
        rlTexCoord2f(0.0f, 1.0f);
        rlVertex2f(x, y);

    rlEnd();
}

inline void RenderTarget::blitAspectExpand(GLuint fbTarget, GLAttachement attach, bool blitDepth, bool linear) const
{
    GLbitfield mask = 0;

    if (attach != GLAttachement::NONE) mask |= GL_COLOR_BUFFER_BIT;
    if (blitDepth) mask |= GL_DEPTH_BUFFER_BIT;
    if (mask == 0) return;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer.id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbTarget);

    if (mask & GL_COLOR_BUFFER_BIT) {
        glReadBuffer(static_cast<GLenum>(attach));
    }

    glBlitFramebuffer(
        0, 0, mWidth, mHeight,
        0, 0, GetScreenWidth(), GetScreenHeight(),
        mask, linear ? GL_LINEAR : GL_NEAREST
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void RenderTarget::blitAspectKeep(GLuint fbTarget, GLAttachement attach, bool blitDepth, bool linear) const
{
    GLbitfield mask = 0;

    if (attach != GLAttachement::NONE) mask |= GL_COLOR_BUFFER_BIT;
    if (blitDepth) mask |= GL_DEPTH_BUFFER_BIT;
    if (mask == 0) return;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer.id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbTarget);

    if (mask & GL_COLOR_BUFFER_BIT) {
        glReadBuffer(static_cast<GLenum>(attach));
    }

    // Screen size and content to be blitted
    const float screenRatio = static_cast<float>(GetScreenWidth()) / GetScreenHeight();
    const float contentRatio = static_cast<float>(mWidth) / mHeight;

    // Variables for the starting position of the blit and the size of the area to be blitted
    int blitX = 0, blitY = 0, blitWidth = GetScreenWidth(), blitHeight = GetScreenHeight();

    if (contentRatio > screenRatio) {
        // The image is wider than the screen, adjust the width to maintain the aspect ratio
        blitHeight = static_cast<int>(GetScreenWidth() / contentRatio);
        blitY = (GetScreenHeight() - blitHeight) / 2; // Center it vertically
    } else {
        // The image is taller than the screen, adjust the height to maintain the aspect ratio
        blitWidth = static_cast<int>(GetScreenHeight() * contentRatio);
        blitX = (GetScreenWidth() - blitWidth) / 2; // Center it horizontally
    }

    glBlitFramebuffer(
        0, 0, mWidth, mHeight,
        blitX, blitY, blitX + blitWidth, blitY + blitHeight,
        mask, linear ? GL_LINEAR : GL_NEAREST
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


/* Private implementation */

inline GLTexture RenderTarget::createTexture(GLenum target, GLenum internalFormat, GLenum format, GLenum type)
{
    GLTexture tex;
    switch (target) {
        case GL_TEXTURE_2D:
            tex = GLTexture::gen2D(nullptr, mWidth, mHeight, internalFormat, format, type);
            break;
        case GL_TEXTURE_CUBE_MAP:
            tex = GLTexture::genCube(nullptr, mWidth, mHeight, internalFormat, format, type);
            break;
        default:
            assert(false && "Unsupported target texture");
            break;
    }
    tex.filter(GLTexture::Filter::NEAREST);
    tex.wrap(GLTexture::Wrap::CLAMP_BORDER);
    return tex;
}

} // namespace r3d

#endif // R3D_DETAIL_RENDER_TARGET_HPP
