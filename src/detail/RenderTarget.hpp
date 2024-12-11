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

#include "./helper/GL/GLTexture.hpp"
#include "./helper/GL/GLFramebuffer.hpp"

#include <raylib.h>
#include <rlgl.h>

#include <initializer_list>
#include <cassert>
#include <map>

namespace r3d {

// Sert de containeur generique de cible de rendu,
// par ex. pour les shadow maps, les posts process, etc..
class RenderTarget
{
public:
    RenderTarget(int w, int h);

    GLTexture& createAttachment(GLAttachement attach, GLenum target, GLenum internalFormat, GLenum format, GLenum type);

    void setDrawBuffers(const std::initializer_list<GLAttachement>& list) const;

    void setDrawBuffer(GLAttachement attach) const;

    void setReadBuffer(GLAttachement attach) const;

    void begin() const;

    static void end();

    void bindFace(GLAttachement attach, int face) const;

    GLTexture& attachement(GLAttachement attach);

    const GLTexture& attachement(GLAttachement attach) const;

    // Permet de redimensionner les attachements de la cible de rendu
    void resize(int newWidth, int newHeight);

    int width() const;

    int height() const;

    int texelWidth() const;

    int texelHeight() const;

    void draw(GLAttachement attach, int x, int y, int w, int h) const;

    // Blit l'integralité du buffer (profondeur et attachement indiqué, COLOR_0 par defaut) au framebuffer principal de maniere entendu en utilisant l'apect ratio de la fenetre
    void blitAspectExpand(GLAttachement attach = GLAttachement::COLOR_0, bool blitDepth = true, bool linear = false) const;

    // Blit l'integralité du buffer (profondeur et attachement indiqué, COLOR_0 par defaut) au framebuffer principal en conservant l'aspect ratio de la resolution interne
    void blitAspectKeep(GLAttachement attach = GLAttachement::COLOR_0, bool blitDepth = true, bool linear = false) const;

private:
    std::map<GLAttachement, GLTexture> mAttachments; ///< TODO: Faire un allocateur de pile
    GLFramebuffer mFramebuffer;
    int mWidth, mHeight;
    float mTxlW, mTxlH;

    // Permet de généré des textures, utilisé pour la creation d'attachement
    // NOTE: Ne supporte uniquement 'GL_TEXTURE_2D' et 'GL_TEXTURE_CUBE_MAP'
    GLTexture createTexture(GLenum target, GLenum internalFormat, GLenum format, GLenum type);

    // Permet de convertir l'enum class 'GLAttachement' en type d'attachement OpenGL
    static GLenum glAttachement(GLAttachement attach);
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

// Permet de redimensionner les attachements de la cible de rendu
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

inline int RenderTarget::width() const
{
    return mWidth;
}

inline int RenderTarget::height() const
{
    return mHeight;
}

inline int RenderTarget::texelWidth() const
{
    return mTxlW;
}

inline int RenderTarget::texelHeight() const
{
    return mTxlH;
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

inline void RenderTarget::blitAspectExpand(GLAttachement attach, bool blitDepth, bool linear) const
{
    GLbitfield mask = 0;

    if (attach != GLAttachement::NONE) mask |= GL_COLOR_BUFFER_BIT;
    if (blitDepth) mask |= GL_DEPTH_BUFFER_BIT;
    if (mask == 0) return;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer.id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

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

inline void RenderTarget::blitAspectKeep(GLAttachement attach, bool blitDepth, bool linear) const
{
    GLbitfield mask = 0;

    if (attach != GLAttachement::NONE) mask |= GL_COLOR_BUFFER_BIT;
    if (blitDepth) mask |= GL_DEPTH_BUFFER_BIT;
    if (mask == 0) return;

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer.id());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    if (mask & GL_COLOR_BUFFER_BIT) {
        glReadBuffer(static_cast<GLenum>(attach));
    }

    // Taille de l'écran et du contenu à bliter
    const float screenRatio = static_cast<float>(GetScreenWidth()) / GetScreenHeight();
    const float contentRatio = static_cast<float>(mWidth) / mHeight;

    // Variables pour la position de départ du blit et la taille de la zone à bliter
    int blitX = 0, blitY = 0, blitWidth = GetScreenWidth(), blitHeight = GetScreenHeight();

    if (contentRatio > screenRatio) {
        // L'image est plus large que l'écran, on ajuste la largeur pour conserver le ratio d'aspect
        blitHeight = static_cast<int>(GetScreenWidth() / contentRatio);
        blitY = (GetScreenHeight() - blitHeight) / 2; // On centre verticalement
    } else {
        // L'image est plus haute que l'écran, on ajuste la hauteur pour conserver le ratio d'aspect
        blitWidth = static_cast<int>(GetScreenHeight() * contentRatio);
        blitX = (GetScreenWidth() - blitWidth) / 2; // On centre horizontalement
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
