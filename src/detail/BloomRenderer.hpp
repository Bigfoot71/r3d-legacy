#ifndef R3D_DETAIL_BLOOM_RENDERER_HPP
#define R3D_DETAIL_BLOOM_RENDERER_HPP

#include "detail/ShaderCodes.hpp"
#include "./GL/GLFramebuffer.hpp"
#include "./RenderTarget.hpp"
#include "./GL/GLShader.hpp"
#include "./Quad.hpp"
#include <array>

namespace r3d {

/**
 * @brief Class for rendering the bloom effect in a scene.
 * 
 * This class handles the process of applying a bloom effect by blurring the bright areas of the scene and combining them with the original image.
 */
class BloomRenderer
{
public:
    /**
     * @brief Constructs a BloomRenderer with specified dimensions.
     * @param rendererWidth The width of the renderer.
     * @param rendererHeight The height of the renderer.
     */
    BloomRenderer(int rendererWidth, int rendererHeight);

    /**
     * @brief Resizes the renderer.
     * @param newWidth The new width for the renderer.
     * @param newHeight The new height for the renderer.
     */
    void resize(int newWidth, int newHeight);

    /**
     * @brief Renders the bloom effect using the given luminance texture.
     * @param texSceneLum The texture containing the scene luminance for bloom.
     * @param iterations The number of blur iterations to perform.
     */
    void render(const GLTexture& texSceneLum, int iterations);

    /**
     * @brief Returns the final result of the bloom effect.
     * @return The texture containing the result of the bloom effect.
     */
    const GLTexture& result() const;

private:
    std::array<RenderTarget, 2> mTargets;   /**< Render targets for the bloom effect. */
    GLShader mShaderBlur;                   /**< Shader used for the blur effect. */
    Quad mQuad;                             /**< A quad used for rendering the texture. */

    bool mHorizontalPass;                   /**< Flag indicating whether the current pass is horizontal or vertical for the blur. */
};

/* Implementation */

inline BloomRenderer::BloomRenderer(int rendererWidth, int rendererHeight)
    : mTargets({
        RenderTarget(rendererWidth / 2, rendererHeight / 2),
        RenderTarget(rendererWidth / 2, rendererHeight / 2)
    })
    , mShaderBlur(VS_CODE_BLUR, FS_CODE_BLUR)
{
    for (int i = 0; i < 2; i++) {
        auto& texture = mTargets[i].createAttachment(
            GLAttachement::COLOR_0, GL_TEXTURE_2D,
            GL_RGBA16F, GL_RGBA, GL_FLOAT
        );
        texture.filter(GLTexture::Filter::TRILINEAR);
        texture.wrap(GLTexture::Wrap::CLAMP_BORDER);
        texture.genMipmaps();
    }
}

inline void BloomRenderer::resize(int newWidth, int newHeight)
{
    for (auto& target : mTargets) {
        target.resize(newWidth / 2, newHeight / 2);
    }
}

inline void BloomRenderer::render(const GLTexture& texSceneLum, int iterations)
{
    mHorizontalPass = true;

    mShaderBlur.begin();
    {
        for (int i = 0; i < iterations; i++, mHorizontalPass = !mHorizontalPass) {
            mTargets[mHorizontalPass].begin();
            {
                mShaderBlur.setValue("uDirection", mHorizontalPass
                    ? Vector2 { 1, 0 } : Vector2 { 0, 1 }
                );
                mShaderBlur.bindTexture("uTexture", i > 0
                    ? mTargets[!mHorizontalPass].attachement(GLAttachement::COLOR_0)
                    : texSceneLum
                );
                mQuad.draw();
            }
            mShaderBlur.unbindTextures();
        }
        GLFramebuffer::unbind();
    }
    mShaderBlur.end();
}

inline const GLTexture& BloomRenderer::result() const
{
    return mTargets[!mHorizontalPass].attachement(GLAttachement::COLOR_0);
}

}

#endif // R3D_DETAIL_BLOOM_RENDERER_HPP
