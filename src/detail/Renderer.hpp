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

#ifndef R3D_DETAIL_RENDERER_HPP
#define R3D_DETAIL_RENDERER_HPP

#include "r3d.h"

#include "detail/ShaderCodes.hpp"

#include "./helper/GL/GLFramebuffer.hpp"
#include "./helper/RL/RLCamera3D.hpp"
#include "./helper/RL/RLTexture.hpp"
#include "./helper/RL/RLShader.hpp"

#include "./helper/GL/GLTexture.hpp"
#include "./helper/GL/GLShader.hpp"
#include "./helper/GL/GLQuad.hpp"

#include "./utils/Frustum.hpp"
#include "./utils/IDMan.hpp"
#include "./utils/Math.hpp"

#include "./RenderTarget.hpp"
#include "./BatchMap.hpp"
#include "./Skybox.hpp"
#include "./Model.hpp"
#include "./Light.hpp"
#include "./GL.hpp"
#include "rlgl.h"

#include <raylib.h>
#include <raymath.h>

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <cstdio>
#include <cfloat>
#include <map>

namespace r3d {

/**
 * @brief Maximum number of lights per surface, matching the value defined in the 'material' shader.
 * @note If you modify this value, ensure to update it in 'material.vs' and 'material.fs' shaders as well.
 */
static constexpr int SHADER_LIGHT_COUNT = 8;

/**
 * @brief Stores a draw call for rendering in shadow maps.
 */
struct DrawCallShadow
{
    const R3D_Surface *surface; ///< Pointer to the surface to be rendered.
    const Matrix transform;     ///< Transformation matrix for the draw call.
};

/**
 * @brief Stores a draw call for rendering in the main scene.
 */
struct DrawCallScene
{
    /**
     * @brief Alias for an array of light pointers used in the shader.
     */
    using LightsArray = std::array<const Light*, SHADER_LIGHT_COUNT>;

    struct {
        const Mesh *mesh;           ///< Pointer to the mesh to be rendered.
        R3D_Material material;      ///< Material copy for rendering the same mesh with different parameters.
    } surface;                      ///< Surface information for the draw call.

    const Matrix transform;         ///< Transformation matrix for the draw call.
    LightsArray lights;             ///< Array of light pointers influencing this draw call.
};

/**
 * @brief Custom hasher for the unordered_map used to store shaders.
 * 
 * The key used is `R3D_MaterialShaderConfig`, enabling sorting of shaders based on their features.
 */
struct MaterialShaderConfigHash {
    size_t operator()(const R3D_MaterialShaderConfig& config) const {
        return std::hash<uint32_t>()(*reinterpret_cast<const uint32_t*>(&config));
    }
};

/**
 * @brief Equality comparator for the unordered_map used to store shaders.
 * 
 * Compares two `R3D_MaterialShaderConfig` objects to determine equality.
 */
struct MaterialShaderConfigEqual {
    bool operator()(const R3D_MaterialShaderConfig& lhs, const R3D_MaterialShaderConfig& rhs) const {
        return *reinterpret_cast<const uint32_t*>(&lhs) == *reinterpret_cast<const uint32_t*>(&rhs);
    }
};

} // namespace r3d

/**
 * @brief Less-than operator for sorting material configurations.
 * 
 * Used for sorting objects by material in a map, minimizing OpenGL state changes during rendering.
 * 
 * @param lhs Left-hand side material configuration.
 * @param rhs Right-hand side material configuration.
 * @return True if `lhs` is less than `rhs`, false otherwise.
 */
inline bool operator<(R3D_MaterialConfig lhs, R3D_MaterialConfig rhs) {
    return *reinterpret_cast<const uint64_t*>(&lhs) < *reinterpret_cast<const uint64_t*>(&rhs);
}

namespace r3d {

/**
 * @brief The Renderer class handles the rendering pipeline, including drawing models, managing lights, and rendering shadow maps.
 */
class Renderer
{
public:
    R3D_Environment environment;    ///< Environment settings for the renderer.
    bool performFrustumCulling;     ///< Flag to specify whether frustum culling should be performed for visible scene objects.
    bool blitAspectKeep;            ///< Flag to maintain aspect ratio during blitting.
    bool blitLinear;                ///< Flag to enable linear blitting.

public:
    /**
     * @brief Initializes the Renderer with internal framebuffer dimensions and optional flags.
     * 
     * @param internalWidth Width of the internal framebuffer. Specify a value `<= 0` to use the main framebuffer dimensions.
     * @param internalHeight Height of the internal framebuffer. Specify a value `<= 0` to use the main framebuffer dimensions.
     * @param flags Additional flags to configure the renderer.
     */
    Renderer(int internalWidth, int internalHeight, int flags);

    /**
     * @brief Loads and compiles the specified material configuration.
     * 
     * @param config The material configuration to load.
     */
    void loadMaterialConfig(R3D_MaterialConfig config);

    /**
     * @brief Unloads the specified material configuration.
     * 
     * @param config The material configuration to unload.
     */
    void unloadMaterialConfig(R3D_MaterialConfig config);

    /**
     * @brief Checks if the specified material configuration is valid.
     * 
     * @param config The material configuration to check.
     * @return True if the material configuration is valid, false otherwise.
     */
    bool isMaterialConfigValid(R3D_MaterialConfig config) const;

    /**
     * @brief Sets the camera for the renderer and calculates its view matrix, projection matrix, and frustum.
     * 
     * This function performs a copy of the provided camera and calculates its view and projection matrices. 
     * It also computes the frustum based on the camera's view and projection. These calculations are necessary 
     * for performing frustum culling tests during each subsequent call to the `draw` method.
     * 
     * @param camera The camera to use for rendering. It is copied and used for view and projection calculations.
     */
    void setCamera(const Camera3D& camera);

    /**
     * @brief Evaluates and queues a model for rendering based on visibility and rendering contexts.
     * 
     * This method performs various tests to determine if the given model should be rendered. It applies frustum culling 
     * to check if the model is visible within the camera's frustum, verifies if the model needs to be rendered in shadow maps, 
     * and includes the model in the appropriate rendering batches for contexts such as the main scene or shadow maps.
     * 
     * @param model The model to evaluate and queue for rendering.
     * @param position The world-space position of the model.
     * @param rotationAxis The axis around which the model is rotated.
     * @param rotationAngle The angle of rotation, in degrees.
     * @param scale The scale of the model.
     */
    void draw(const R3D_Model& model, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector3& scale);

    /**
     * @brief Executes the rendering of all queued surfaces and presents the final frame to the display.
     * 
     * This method processes all surfaces that were queued by the `draw` method during the current frame. It performs the actual rendering 
     * of these surfaces in their respective contexts (e.g., shadow maps, main scene) and applies any post-processing effects before 
     * presenting the final output to the display.
     */
    void present();

    /**
     * @brief Updates the internal resolution of the renderer.
     * 
     * @param newWidth The new internal width.
     * @param newHeight The new internal height.
     */
    void updateInternalResolution(int newWidth, int newHeight);

    /**
     * @brief Adds a light to the scene.
     * 
     * @param type The type of light to add.
     * @param shadowMapResolution Resolution of the shadow map for the light.
     * @return An ID representing the added light.
     */
    R3D_Light addLight(R3D_LightType type, int shadowMapResolution);

    /**
     * @brief Removes a light from the scene.
     * 
     * @param id The ID of the light to remove.
     */
    void removeLight(R3D_Light id);

    /**
     * @brief Retrieves the data of a light by its ID (const version).
     * 
     * @param id The ID of the light.
     * @return A constant reference to the light's data.
     */
    const Light& getLight(R3D_Light id) const;

    /**
     * @brief Retrieves the data of a light by its ID.
     * 
     * @param id The ID of the light.
     * @return A reference to the light's data.
     */
    Light& getLight(R3D_Light id);

    /**
     * @brief Retrieves the default material configuration used by the renderer.
     * 
     * @return The default material configuration.
     */
    R3D_MaterialConfig getDefaultMaterialConfig() const;

    /**
     * @brief Sets the default material configuration for the renderer.
     * 
     * @param config The material configuration to set as default.
     */
    void setDefaultMaterialConfig(R3D_MaterialConfig config);

    /**
     * @brief Retrieves a black texture used as a placeholder or default texture.
     * 
     * @return A constant reference to the black texture.
     */
    const Texture2D& getTextureBlack() const;

    /**
     * @brief Retrieves a white texture used as a placeholder or default texture.
     * 
     * @return A constant reference to the white texture.
     */
    const Texture2D& getTextureWhite() const;

    /**
     * @brief Retrieves the draw call counts for the current frame.
     * 
     * @param sceneDrawCount Pointer to store the count of draw calls for the main scene.
     * @param shadowDrawCount Pointer to store the count of draw calls for shadow maps.
     */
    void getDrawCallCount(int* sceneDrawCount, int* shadowDrawCount) const;

    /**
     * @brief Renders a shadow map for debugging purposes.
     * 
     * @param light The light whose shadow map is to be rendered.
     * @param x The x-coordinate of the shadow map display.
     * @param y The y-coordinate of the shadow map display.
     * @param width The width of the shadow map display.
     * @param height The height of the shadow map display.
     * @param zNear The near clipping plane.
     * @param zFar The far clipping plane.
     */
    void drawShadowMap(R3D_Light light, int x, int y, int width, int height, float zNear, float zFar) const;

private:
    /**
     * @brief Draws a surface in the shadow map render pass.
     * 
     * @param drawCall The draw call information.
     * @param light The light casting shadows.
     */
    void drawSurfaceShadow(const DrawCallShadow& drawCall, const Light& light) const;

    /**
     * @brief Draws a surface in the main scene render pass.
     * 
     * @param drawCall The draw call information.
     * @param shader The shader to use for rendering.
     */
    void drawSurfaceScene(const DrawCallScene& drawCall, const GLShader& shader) const;

    /**
     * @brief Renders the shadow map for a directional light.
     * 
     * @param light The directional light casting shadows.
     * @param batch The batch of shadow draw calls.
     */
    void renderShadowMapDirectional(const Light& light, const std::vector<DrawCallShadow>& batch) const;

    /**
     * @brief Renders the shadow map for a spotlight.
     * 
     * @param light The spotlight casting shadows.
     * @param batch The batch of shadow draw calls.
     */
    void renderShadowMapSpot(const Light& light, const std::vector<DrawCallShadow>& batch) const;

    /**
     * @brief Renders the shadow map for an omnidirectional light.
     * 
     * @param light The omnidirectional light casting shadows.
     * @param batch The batch of shadow draw calls.
     */
    void renderShadowMapOmni(const Light& light, const std::vector<DrawCallShadow>& batch) const;

private:
    int mInternalWidth;                         ///< Internal framebuffer width.
    int mInternalHeight;                        ///< Internal framebuffer height.
    RenderTarget mTargetScene;                  ///< Render target for the main scene.
    RenderTarget mTargetPostFX;                 ///< Render target for post-processing effects.
    std::array<RenderTarget, 2> mTargetBlur;    ///< Render targets for blurring effects.

    std::unordered_map<
        R3D_MaterialShaderConfig, GLShader,
        MaterialShaderConfigHash, MaterialShaderConfigEqual
    > mShaderMaterials; ///< Shader map for material properties.

    BatchMap<R3D_MaterialConfig, DrawCallScene> mSceneBatches;  ///< Scene draw calls sorted by material.
    BatchMap<R3D_Light, DrawCallShadow> mShadowBatches;         ///< Shadow draw calls for each light.

    std::map<R3D_Light, Light> mLights;         ///< Map of lights and their data.
    R3D_MaterialConfig mDefaultMaterialConfig;  ///< Default material configuration.
    IDMan<R3D_Light> mLightIDMan;               ///< Light ID manager.

    RLTexture mBlackTexture2D;      ///< Black placeholder texture.
    RLTexture mWhiteTexture2D;      ///< White placeholder texture.
    GLQuad mQuad;                   ///< Quad used for rendering.

    GLShader mShaderBlur;           ///< Shader for blurring.
    GLShader mShaderPostFX;         ///< Shader for post-processing effects.

    RLShader mShaderDepthCube;      ///< Shader for cube depth rendering.
    RLShader mShaderDepth;          ///< Shader for depth rendering.

    RLCamera3D mCamera;             ///< Camera for rendering.
    Matrix mMatCameraView;          ///< View matrix for the camera.
    Matrix mMatCameraProj;          ///< Projection matrix for the camera.
    Frustum mFrustumCamera;         ///< Camera frustum.

    std::optional<GLShader> mDebugShaderDepthTexture2D; ///< Debug shader for 2D depth textures.
    std::optional<GLShader> mDebugShaderDepthCubemap;   ///< Debug shader for cubemap depth textures.
};


/* Public implementation */

inline Renderer::Renderer(int internalWidth, int internalHeight, int flags)
    : mInternalWidth(internalWidth > 0 ? internalWidth :  rlGetFramebufferWidth())
    , mInternalHeight(internalHeight > 0 ? internalHeight : rlGetFramebufferHeight())
    , environment({
        .bloom {
            .mode           = R3D_BLOOM_DISABLED,
            .intensity      = 1.0f,
            .hdrThreshold   = 1.0f,
            .iterations     = 10
        },
        .fog {
            .mode           = R3D_FOG_DISABLED,
            .color          = GRAY,
            .start          = 10.0f,
            .end            = 30.0f,
            .density        = 0.1f
        },
        .tonemap {
            .mode           = R3D_TONEMAP_LINEAR,
            .exposure       = 1.0f,
            .white          = 1.0f
        },
        .adjustements {
            .brightness     = 1.0f,
            .contrast       = 1.0f,
            .saturation     = 1.0f
        },
        .world {
            .skybox         = nullptr,
            .background     = DARKGRAY,
            .ambient        = DARKGRAY
        }
    })
    , performFrustumCulling(!(flags & R3D_FLAG_NO_FRUSTUM_CULLING))
    , blitAspectKeep(flags & R3D_FLAG_ASPECT_KEEP)
    , blitLinear(flags & R3D_FLAG_BLIT_LINEAR)
    , mTargetScene(mInternalWidth, mInternalHeight)
    , mTargetPostFX(mInternalWidth, mInternalHeight)
    , mTargetBlur({
        RenderTarget(mInternalWidth, mInternalHeight),
        RenderTarget(mInternalWidth, mInternalHeight)
    })
    , mDefaultMaterialConfig({
        .shader = {
            .diffuse = R3D_DIFFUSE_BURLEY,
            .specular = R3D_SPECULAR_SCHLICK_GGX,
            .reserved = 0,
            .flags = R3D_MATERIAL_FLAG_RECEIVE_SHADOW
                   | R3D_MATERIAL_FLAG_SKY_IBL
        },
        .blendMode = R3D_BLEND_ALPHA,
        .cullMode = R3D_CULL_BACK,
        .reserved1 = 0,
        .reserved2 = 0
    })
    , mBlackTexture2D(BLACK)
    , mWhiteTexture2D(WHITE)
    , mShaderBlur(VS_CODE_BLUR, FS_CODE_BLUR)
    , mShaderPostFX(VS_CODE_POSTFX, FS_CODE_POSTFX)
    , mShaderDepthCube(VS_CODE_DEPTH_CUBE, FS_CODE_DEPTH_CUBE)
    , mShaderDepth(VS_CODE_DEPTH, FS_CODE_DEPTH)
{
    // Managing initialization attributes

    if (flags & R3D_FLAG_DEBUG_SHADOW_MAP) {
        mDebugShaderDepthTexture2D.emplace(VS_CODE_DEBUG_DEPTH, FS_CODE_DEBUG_DEPTH_TEXTURE_2D);
        mDebugShaderDepthCubemap.emplace(VS_CODE_DEBUG_DEPTH, FS_CODE_DEBUG_DEPTH_CUBEMAP);
    }

    // Setup of some shaders

    mShaderDepthCube.locs[SHADER_LOC_VECTOR_VIEW] = mShaderDepthCube.location("viewPos");

    // Setup the default material

    loadMaterialConfig(mDefaultMaterialConfig);

    // Configuring the scene render target

    mTargetScene.createAttachment(GLAttachement::DEPTH, GL_TEXTURE_2D, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
    mTargetScene.createAttachment(GLAttachement::COLOR_0, GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    mTargetScene.createAttachment(GLAttachement::COLOR_1, GL_TEXTURE_2D, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    mTargetScene.setDrawBuffers({
        GLAttachement::COLOR_0,
        GLAttachement::COLOR_1,
    });

    // Configuring the post effects render target

    mTargetPostFX.createAttachment(
        GLAttachement::COLOR_0, GL_TEXTURE_2D,
        GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE
    );

    // Configuring render targets for two-pass blur processing (used for bloom)

    for (int i = 0; i < 2; i++) {
        auto& texture = mTargetBlur[i].createAttachment(
            GLAttachement::COLOR_0, GL_TEXTURE_2D,
            GL_RGBA16F, GL_RGBA, GL_FLOAT
        );
        texture.filter(GLTexture::Filter::BILINEAR);
        texture.wrap(GLTexture::Wrap::CLAMP_BORDER);
    }
}

inline void Renderer::loadMaterialConfig(R3D_MaterialConfig config)
{
    // Instantiates an entity vector in 'm_material_batches' for this material type if it has not already been created

    if (!mSceneBatches.isBatchExist(config)) {
        mSceneBatches.addBatch(config);
    }

    // Check if the shader has already been created for these properties

    const auto it_material_shader = mShaderMaterials.find(config.shader);

    if (it_material_shader != mShaderMaterials.cend()) {
        return;
    }

    // Constructs the shader code for the given properties

    std::string vsCode("#version 330 core\n");
    {
        if (config.shader.flags & R3D_MATERIAL_FLAG_VERTEX_COLOR) {
            vsCode += "#define VERTEX_COLOR\n";
        }
        if (config.shader.diffuse == R3D_DIFFUSE_UNSHADED) {
            vsCode += "#define DIFFUSE_UNSHADED\n";
        } else {
            if (config.shader.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
                vsCode += "#define RECEIVE_SHADOW\n";
            }
            if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
                vsCode += "#define MAP_NORMAL\n";
            }
        }

        vsCode += VS_CODE_MATERIAL;
    }

    std::string fsCode("#version 330 core\n");
    {
        switch (config.shader.diffuse) {
            case R3D_DIFFUSE_UNSHADED:
                fsCode += "#define DIFFUSE_UNSHADED\n";
                break;
            case R3D_DIFFUSE_BURLEY:
                fsCode += "#define DIFFUSE_BURLEY\n";
                break;
            case R3D_DIFFUSE_DISNEY:
                fsCode += "#define DIFFUSE_DISNEY\n";
                break;
            case R3D_DIFFUSE_LAMBERT:
                fsCode += "#define DIFFUSE_LAMBERT\n";
                break;
            case R3D_DIFFUSE_PHONG:
                fsCode += "#define DIFFUSE_PHONG\n";
                break;
            case R3D_DIFFUSE_TOON:
                fsCode += "#define DIFFUSE_TOON\n";
                break;
        }

        if (config.shader.flags & R3D_MATERIAL_FLAG_VERTEX_COLOR) {
            fsCode += "#define VERTEX_COLOR\n";
        }

        if (config.shader.diffuse != R3D_DIFFUSE_UNSHADED) {
            switch (config.shader.specular) {
                case R3D_SPECULAR_SCHLICK_GGX:
                    fsCode += "#define SPECULAR_SCHLICK_GGX\n";
                    break;
                case R3D_SPECULAR_DISNEY:
                    fsCode += "#define SPECULAR_DISNEY\n";
                    break;
                case R3D_SPECULAR_BLINN_PHONG:
                    fsCode += "#define SPECULAR_BLINN_PHONG\n";
                    break;
                case R3D_SPECULAR_TOON:
                    fsCode += "#define SPECULAR_TOON\n";
                    break;
                default:
                    break;
            }
            if (config.shader.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
                fsCode += "#define RECEIVE_SHADOW\n";
            }
            if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_EMISSION) {
                fsCode += "#define MAP_EMISSION\n";
            }
            if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
                fsCode += "#define MAP_NORMAL\n";
            }
            if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_AO) {
                fsCode += "#define MAP_AO\n";
            }
            if (config.shader.flags & R3D_MATERIAL_FLAG_SKY_IBL) {
                fsCode += "#define SKY_IBL\n";
            }
        }

        fsCode += FS_CODE_MATERIAL;
    }

    mShaderMaterials.emplace(config.shader,
        GLShader(vsCode, fsCode)
    );
}

inline void Renderer::unloadMaterialConfig(R3D_MaterialConfig config)
{
    mSceneBatches.eraseBatch(config);

    auto it_material_shader = mShaderMaterials.find(config.shader);

    if (it_material_shader != mShaderMaterials.end()) {
        mShaderMaterials.erase(it_material_shader);
    }
}

inline bool Renderer::isMaterialConfigValid(R3D_MaterialConfig config) const
{
    return (mShaderMaterials.find(config.shader) != mShaderMaterials.cend());
}

inline void Renderer::setCamera(const Camera3D& camera)
{
    mCamera = camera;

    // Calculates the desired aspect ratio for the projection matrix

    float aspect = 1.0f;
    if (blitAspectKeep) {
        aspect = static_cast<float>(mInternalWidth) / mInternalHeight;
    } else {
        aspect = static_cast<float>(GetScreenWidth()) / GetScreenHeight();
    }

    // Retrieves the camera's view and projection matrices

    mMatCameraView = mCamera.viewMatrix();
    mMatCameraProj = mCamera.projMatrix(aspect);

    // Computes the camera's frustum if necessary

    if (performFrustumCulling) {
        mFrustumCamera = Frustum(MatrixMultiply(mMatCameraView, mMatCameraProj));
    }
}

inline void Renderer::draw(const R3D_Model& model, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector3& scale)
{
    const std::vector<R3D_Surface>& surfaces = static_cast<Model*>(model.internal)->surfaces;

    // Computes the model's transformation matrix using the provided parameters,
    // the transformation assigned to the model, any parent transformations if present,
    // the transformation specified via rlgl, and also retrieves the model's actual global position.

    Matrix matModel = MatrixMultiply(
        MatrixMultiply(
            MatrixScale(scale.x, scale.y, scale.z),
            MatrixRotate(rotationAxis, rotationAngle * DEG2RAD)
        ),
        MatrixTranslate(position.x, position.y, position.z)
    );

    matModel = MatrixMultiply(matModel, R3D_TransformToGlobal(&model.transform));
    matModel = MatrixMultiply(matModel, rlGetMatrixTransform());

    const Vector3 modelPosition = Vector3Add(model.transform.position, position);

    // Retrieves the model's bounding box transformed according to the previously obtained transformation matrix.

    BoundingBox globalAABB = model.aabb;
    globalAABB.min = Vector3Transform(globalAABB.min, matModel);
    globalAABB.max = Vector3Transform(globalAABB.max, matModel);

    // Performs a bounding box test for the model against the camera's frustum if necessary
    // to determine if the object should be rendered in the visible scene. This feature can be
    // disabled using the initialization flag 'R3D_FLAG_NO_FRUSTUM_CULLING'.

    bool drawSurfaceScene = true;
    if (performFrustumCulling && model.shadow != R3D_CAST_SHADOW_ONLY) {
        drawSurfaceScene = mFrustumCamera.aabbIn(globalAABB);
    }

    // Determines which lights will interact with the model.
    // This part checks which lights should cast shadows onto the model
    // and also associates the lights with the draw call if the model is to be rendered in the scene.

    DrawCallScene::LightsArray lightsWhichShouldIlluminate{};
    int lightsWhichShouldIlluminateCount = 0;

    for (const auto& [id, light] : mLights) {

        if (!light.enabled) continue;

        // Here, we use a rather naive approach. If the light is not directional—
        // for instance, it does not simulate the sun—then we check whether the squared 
        // distance between the origin of the model and the light exceeds the maximum range 
        // of the light's effective field. If it does, we can safely omit this light source for the model.

        // Using the squared distance is faster to compute and provides a certain margin, 
        // making it acceptable in this context.

        const float lightMaxDistSqr = light.maxDistance * light.maxDistance;
        if (light.type != R3D_DIRLIGHT && Vector3DistanceSqr(modelPosition, light.position) > lightMaxDistSqr) {
            continue;
        }

        // Here we add:
        //   - 1: The surfaces to the shadow batch for the light
        //   - 2: The light to the surface draw calls for the scene rendering

        auto& batch = mShadowBatches.getBatch(id);

        switch (light.type) {
            case R3D_DIRLIGHT: {
                if (light.frustum.aabbIn(globalAABB)) {
                    if (model.shadow != R3D_CAST_OFF && light.shadow) {
                        for (const auto& surface : surfaces) {
                            batch.push_back(DrawCallShadow { &surface, matModel });
                        }
                    }
                    if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                        lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                        lightsWhichShouldIlluminateCount++;
                    }
                }
            } break;
            case R3D_SPOTLIGHT: {
                if (light.frustum.aabbIn(globalAABB)) {
                    if (model.shadow != R3D_CAST_OFF && light.shadow) {
                        for (const auto& surface : surfaces) {
                            batch.push_back(DrawCallShadow { &surface, matModel });
                        }
                    }
                    if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                        lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                        lightsWhichShouldIlluminateCount++;
                    }
                }
            } break;
            case R3D_OMNILIGHT: {
                // If we have reached this point, it means that the model is within the maximum influence distance of the omni light. 
                // Therefore, we don't need to perform a frustum test since it illuminates in all directions.
                if (model.shadow != R3D_CAST_OFF && light.shadow) {
                    for (const auto& surface : surfaces) {
                        batch.push_back(DrawCallShadow { &surface, matModel });
                    }
                }
                if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                    lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                    lightsWhichShouldIlluminateCount++;
                }
            } break;
        }
    }

    if (drawSurfaceScene) {
        for (const auto& surface : surfaces) {
            auto& batch = mSceneBatches.getBatch(surface.material.config);
            batch.push_back(DrawCallScene {
                { .mesh = &surface.mesh, .material = surface.material },
                matModel, lightsWhichShouldIlluminate
            });
        }
    }
}

inline void Renderer::present()
{
    rlDrawRenderBatchActive();
    rlEnableDepthTest();

    /* Shadow casting */

    rlDisableColorBlend();  ///< We deactivate the mixing of colors because the omni light registers the distances in a color attachment

    for (auto& [lightID, batch] : mShadowBatches) {
        if (batch.empty()) continue;

        const auto& light = mLights.at(lightID);

        switch (light.type) {
            case R3D_DIRLIGHT:
                renderShadowMapDirectional(light, batch);
                break;
            case R3D_SPOTLIGHT:
                renderShadowMapSpot(light, batch);
                break;
            case R3D_OMNILIGHT:
                renderShadowMapOmni(light, batch);
                break;
        }

        batch.clear();
    }

    /* Render scene */

    mTargetScene.begin();
    {
        /* Setup view port and clear target scene framebuffer */

        if (blitAspectKeep) {
            glViewport(0, 0, mInternalWidth, mInternalHeight);
        }

        if (environment.world.skybox == nullptr) {
            Vector4 nCol = ColorNormalize(environment.world.background);
            glClearColor(nCol.x, nCol.y, nCol.y, nCol.z);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* Setup view / projection matrices */

        rlMatrixMode(RL_PROJECTION);
        rlPushMatrix();
        rlLoadIdentity();
        rlMultMatrixf(MatrixToFloat(mMatCameraProj));

        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();
        rlMultMatrixf(MatrixToFloat(mMatCameraView));

        /* Render skybox */

        if (environment.world.skybox != nullptr) {
            static_cast<Skybox*>(environment.world.skybox)->draw();
        }

        /* Render surfaces */

        for (auto& [config, batch] : mSceneBatches) {
            if (batch.empty()) continue;

            const r3d::GLShader& shader = mShaderMaterials.at(config.shader);

            // TODO: Find a method to reduce calls to state changes, even if probably ignored by most drivers...
            switch (config.blendMode) {
                case R3D_BLEND_DISABLED:
                    rlDisableColorBlend();
                    break;
                case R3D_BLEND_ALPHA:
                    rlEnableColorBlend();
                    rlSetBlendMode(RL_BLEND_ALPHA);
                    break;
                case R3D_BLEND_ADDITIVE:
                    rlEnableColorBlend();
                    rlSetBlendMode(RL_BLEND_ADDITIVE);
                    break;
                case R3D_BLEND_MULTIPLIED:
                    rlEnableColorBlend();
                    rlSetBlendMode(RL_BLEND_MULTIPLIED);
                    break;
                case R3D_BLEND_ADD_COLORS:
                    rlEnableColorBlend();
                    rlSetBlendMode(RL_BLEND_ADD_COLORS);
                    break;
                case R3D_BLEND_SUBTRACT_COLORS:
                    rlEnableColorBlend();
                    rlSetBlendMode(RL_BLEND_SUBTRACT_COLORS);
                    break;
                case R3D_BLEND_ALPHA_PREMULTIPLY:
                    rlEnableColorBlend();
                    rlSetBlendMode(RL_BLEND_ALPHA_PREMULTIPLY);
                    break;
            }

            switch (config.cullMode) {
                case R3D_CULL_DISABLED:
                    rlDisableBackfaceCulling();
                    break;
                case R3D_CULL_FRONT:
                    rlEnableBackfaceCulling();
                    rlSetCullFace(RL_CULL_FACE_FRONT);
                    break;
                case R3D_CULL_BACK:
                    rlEnableBackfaceCulling();
                    rlSetCullFace(RL_CULL_FACE_BACK);
                    break;
            }

            shader.begin();
                for (const auto& drawCall : batch) {
                    drawSurfaceScene(drawCall, shader);
                }
            shader.end();

            batch.clear();
        }

        /* Reset to the default state */

        rlMatrixMode(RL_PROJECTION);
        rlPopMatrix();

        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();

        rlEnableColorBlend();
        rlSetBlendMode(RL_BLEND_ALPHA);

        rlEnableBackfaceCulling();
        rlSetCullFace(RL_CULL_FACE_BACK);
    }
    mTargetScene.end();

    /* Here we are done with 3D, we can deactivate the depth test */

    rlDisableDepthTest();

    /* Apply gaussian blur for bloom if needed */

    bool horizontal = true;
    if (environment.bloom.mode != R3D_BLOOM_DISABLED) {
        mShaderBlur.begin();
        {
            for (int i = 0; i < environment.bloom.iterations; i++) {
                mTargetBlur[horizontal].begin(); 
                mShaderBlur.setValue("uHorizontal", horizontal);
                mShaderBlur.bindTexture("uTexture", i > 0
                    ? mTargetBlur[!horizontal].attachement(GLAttachement::COLOR_0)
                    : mTargetScene.attachement(GLAttachement::COLOR_1)
                ); 
                mQuad.draw();
                horizontal = !horizontal;
                mShaderBlur.unbindTextures();
            }
            GLFramebuffer::unbind();
        }
        mShaderBlur.end();
    }

    /* Apply post effects */

    mTargetPostFX.begin();
        mShaderPostFX.begin();
        {
            mShaderPostFX.setValue("uBloomMode", static_cast<int>(environment.bloom.mode));

            if (environment.bloom.mode != R3D_BLOOM_DISABLED) {
                mShaderPostFX.bindTexture("uTexBloomBlurHDR", mTargetBlur[!horizontal].attachement(GLAttachement::COLOR_0));
                mShaderPostFX.setValue("uBloomIntensity", environment.bloom.intensity);
            }

            mShaderPostFX.setValue("uFogMode", static_cast<int>(environment.fog.mode));

            if (environment.fog.mode == R3D_FOG_LINEAR) {
                mShaderPostFX.setColor("uFogColor", environment.fog.color, false);
                mShaderPostFX.setValue("uFogStart", environment.fog.start);
                mShaderPostFX.setValue("uFogEnd", environment.fog.end);
            } else if (environment.fog.mode != R3D_FOG_DISABLED) {
                mShaderPostFX.setColor("uFogColor", environment.fog.color, false);
                mShaderPostFX.setValue("uFogDensity", environment.fog.density);
            }

            mShaderPostFX.setValue("uTonemapper", static_cast<int>(environment.tonemap.mode));
            mShaderPostFX.setValue("uExposure", environment.tonemap.exposure);
            mShaderPostFX.setValue("uWhite", environment.tonemap.white);

            mShaderPostFX.setValue("uBrightness", environment.adjustements.brightness);
            mShaderPostFX.setValue("uContrast", environment.adjustements.contrast);
            mShaderPostFX.setValue("uSaturation", environment.adjustements.saturation);

            mShaderPostFX.bindTexture("uTexSceneHDR", mTargetScene.attachement(GLAttachement::COLOR_0));
            mShaderPostFX.bindTexture("uTexSceneDepth", mTargetScene.attachement(GLAttachement::DEPTH));

            mShaderPostFX.setValue("uNear", rlGetCullDistanceNear());
            mShaderPostFX.setValue("uFar", rlGetCullDistanceFar());

            mQuad.draw();
        }
        mShaderPostFX.end();
    mTargetPostFX.end();

    /* Blit result to the main framebuffer */

    if (blitAspectKeep) {
        mTargetPostFX.blitAspectKeep(GLAttachement::COLOR_0, false, blitLinear);
        mTargetScene.blitAspectKeep(GLAttachement::NONE, true, false);
    } else {
        mTargetPostFX.blitAspectExpand(GLAttachement::COLOR_0, false, blitLinear);
        mTargetScene.blitAspectExpand(GLAttachement::NONE, true, false);
    }

    /* Reset viewport */

    glViewport(0, 0, GetScreenWidth(), GetScreenHeight());
}

inline void Renderer::updateInternalResolution(int newWidth, int newHeight)
{
    if (newWidth == mInternalWidth && newHeight == mInternalHeight) {
        return;
    }

    if (newWidth <= 0) newWidth = rlGetFramebufferWidth();
    if (newHeight <= 0) newHeight = rlGetFramebufferHeight();

    mInternalWidth = newWidth;
    mInternalHeight = newHeight;

    mTargetScene.resize(newWidth, newHeight);
    mTargetPostFX.resize(newWidth, newHeight);

    for (auto& target : mTargetBlur) {
        target.resize(newWidth, newHeight);
    }
}

inline R3D_Light Renderer::addLight(R3D_LightType type, int shadowMapResolution)
{
    R3D_Light id  = mLightIDMan.generate();
    mShadowBatches.addBatch(id);
    mLights.emplace(id, Light(type, shadowMapResolution));
    return id;
}

inline void Renderer::removeLight(R3D_Light id)
{
    auto it = mLights.find(id);
    if (it != mLights.end()) {
        mLightIDMan.remove(id);
        mLights.erase(it);
    }
}

inline const Light& Renderer::getLight(R3D_Light id) const
{
    return mLights.at(id);
}

inline Light& Renderer::getLight(R3D_Light id)
{
    return mLights.at(id);
}

inline R3D_MaterialConfig Renderer::getDefaultMaterialConfig() const
{
    return mDefaultMaterialConfig;
}

inline void Renderer::setDefaultMaterialConfig(R3D_MaterialConfig config)
{
    mDefaultMaterialConfig = config;
    loadMaterialConfig(config);         ///< Just in case
}

inline const Texture2D& Renderer::getTextureBlack() const
{
    return mBlackTexture2D;
}

inline const Texture2D& Renderer::getTextureWhite() const
{
    return mWhiteTexture2D;
}

inline void Renderer::getDrawCallCount(int* sceneDrawCount, int* shadowDrawCount) const
{
    if (sceneDrawCount != nullptr) {
        *sceneDrawCount = 0;
        for (const auto& [_, batch] : mSceneBatches) {
            *sceneDrawCount += batch.size();
        }
    }

    if (shadowDrawCount != nullptr) {
        *shadowDrawCount = 0;
        for (const auto& [_, batch] : mShadowBatches) {
            *shadowDrawCount += batch.size();
        }
    }
}

inline void Renderer::drawShadowMap(R3D_Light light, int x, int y, int width, int height, float zNear, float zFar) const
{
    if (!mDebugShaderDepthTexture2D.has_value()) {
        return;
    }

    const r3d::Light& l = getLight(light);

    if (!l.map.has_value()) {
        return;
    }

    float xNDC = (2.0f * (x + width * 0.5f)) / GetScreenWidth() - 1.0f;
    float yNDC = 1.0f - (2.0f * (y + height * 0.5f)) / GetScreenHeight();

    float wNDC = static_cast<float>(width) / GetScreenWidth();
    float hNDC = static_cast<float>(height) / GetScreenHeight();

    Matrix matTransform = MatrixMultiply(
        MatrixScale(wNDC, hNDC, 1.0f),
        MatrixTranslate(xNDC, yNDC, 0.0f)
    );

    switch (l.type) {
        case R3D_DIRLIGHT:
        case R3D_SPOTLIGHT: {
            mDebugShaderDepthTexture2D->begin();
            mDebugShaderDepthTexture2D->setValue("uMVP", matTransform);
            mDebugShaderDepthTexture2D->setValue("uNear", zNear);
            mDebugShaderDepthTexture2D->setValue("uFar", zFar);
            mDebugShaderDepthTexture2D->bindTexture("uTexture", l.map->attachement(GLAttachement::DEPTH));
            mQuad.draw();
            mDebugShaderDepthTexture2D->end();
        } break;
        case R3D_OMNILIGHT: {
            mDebugShaderDepthCubemap->begin();
            mDebugShaderDepthCubemap->setValue("uMVP", matTransform);
            mDebugShaderDepthCubemap->setValue("uMaxVal", zFar);
            mDebugShaderDepthCubemap->bindTexture("uCubemap", l.map->attachement(GLAttachement::COLOR_0));
            mQuad.draw();
            mDebugShaderDepthCubemap->end();
        } break;
    }
}


/* Private implementation */

inline void Renderer::drawSurfaceShadow(const DrawCallShadow& drawCall, const Light& light) const
{
    const ::Mesh& mesh = drawCall.surface->mesh;

    Matrix matModel = MatrixMultiply(drawCall.transform, rlGetMatrixTransform());
    Matrix matView = rlGetMatrixModelview();

    Matrix matModelView = MatrixMultiply(matModel, matView);
    Matrix matProjection = rlGetMatrixProjection();

    if (light.type == R3D_OMNILIGHT) {
        rlSetUniform(mShaderDepthCube.locs[SHADER_LOC_VECTOR_VIEW], &light.position, SHADER_UNIFORM_VEC3, 1);
        rlSetUniformMatrix(mShaderDepthCube.locs[SHADER_LOC_MATRIX_MODEL], matModel);
    }

    if (!rlEnableVertexArray(mesh.vaoId)) {
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
        if (mesh.indices != NULL) {
            rlEnableVertexBufferElement(mesh.vboId[6]);
        }
    }

    int eyeCount = rlIsStereoRenderEnabled() ? 2 : 1;

    for (int eye = 0; eye < eyeCount; eye++) {
        ::Matrix matMVP = MatrixIdentity();

        if (eyeCount == 1) {
            matMVP = MatrixMultiply(matModelView, matProjection);
        } else {
            glViewport(eye * light.map->width() / 2, 0, light.map->width() / 2, light.map->height());
            matMVP = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        if (light.type == R3D_OMNILIGHT) {
            rlSetUniformMatrix(mShaderDepthCube.locs[SHADER_LOC_MATRIX_MVP], matMVP);
        } else {
            rlSetUniformMatrix(mShaderDepth.locs[SHADER_LOC_MATRIX_MVP], matMVP);
        }

        if (mesh.indices != nullptr) {
            rlDrawVertexArrayElements(0, 3 * mesh.triangleCount, 0);
        } else {
            rlDrawVertexArray(0, mesh.vertexCount);
        }
    }

    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
}

inline void Renderer::drawSurfaceScene(const DrawCallScene& drawCall, const GLShader& shader) const
{
    const ::Mesh& mesh = *drawCall.surface.mesh;

    const R3D_Material& material = drawCall.surface.material;
    const R3D_MaterialConfig& config = material.config;

    Matrix matModel = MatrixMultiply(drawCall.transform, rlGetMatrixTransform());
    Matrix matView = rlGetMatrixModelview();

    Matrix matModelView = MatrixMultiply(matModel, matView);
    Matrix matProjection = rlGetMatrixProjection();

    shader.bindTexture("uTexAlbedo", GL_TEXTURE_2D, material.albedo.texture.id);
    shader.setColor("uColAlbedo", material.albedo.color, true);

    if (config.shader.diffuse != R3D_DIFFUSE_UNSHADED) {
        shader.setValue("uMatModel", matModel);
        shader.setValue("uMatNormal", MatrixTranspose(MatrixInvert(matModel)));

        shader.setValue("uViewPos", mCamera.position);

        Skybox* skybox = static_cast<Skybox*>(environment.world.skybox);

        if (config.shader.flags & R3D_MATERIAL_FLAG_SKY_IBL) {
            shader.setValue("uHasSkybox", skybox != nullptr);
            if (skybox != nullptr) {
                shader.bindTexture("uCubeIrradiance", GL_TEXTURE_CUBE_MAP, skybox->getIrradianceCubemapID());
                shader.bindTexture("uCubePrefilter", GL_TEXTURE_CUBE_MAP, skybox->getPrefilterCubemapID());
                shader.bindTexture("uTexBrdfLUT", GL_TEXTURE_2D, skybox->getBrdfLUTTextureID());
            }
        }

        if (skybox == nullptr || !(config.shader.flags & R3D_MATERIAL_FLAG_SKY_IBL)) {
            shader.setColor("uColAmbient", environment.world.ambient, false);
        }

        if (environment.bloom.mode != R3D_BLOOM_DISABLED) {
            shader.setValue("uBloomHdrThreshold", environment.bloom.hdrThreshold);
        }

        if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_EMISSION) {
            shader.bindTexture("uTexEmission", GL_TEXTURE_2D, material.emission.texture.id);
            shader.setColor("uColEmission", material.emission.color, false);
            shader.setValue("uValEmissionEnergy", material.emission.energy);
        }

        if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
            shader.bindTexture("uTexNormal", GL_TEXTURE_2D, material.normal.texture.id);
        }

        if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_AO) {
            shader.bindTexture("uTexAO", GL_TEXTURE_2D, material.ao.texture.id);
            shader.setValue("uValAOLightAffect", material.ao.lightAffect);
        }

        shader.bindTexture("uTexMetalness", GL_TEXTURE_2D, material.metalness.texture.id);
        shader.setValue("uValMetalness", material.metalness.factor);

        shader.bindTexture("uTexRoughness", GL_TEXTURE_2D, material.roughness.texture.id);
        shader.setValue("uValRoughness", material.roughness.factor);

        int lightIndex = 0;

        for (const auto light : drawCall.lights) {
            if (light == nullptr) break;
 
            char lightStr[32] = {};
            std::snprintf(lightStr, sizeof(lightStr), "uLights[%i].%%s", lightIndex);

            shader.setValue(TextFormat(lightStr, "enabled"), true);
            shader.setColor(TextFormat(lightStr, "color"), light->color, false);
            shader.setValue(TextFormat(lightStr, "energy"), light->energy);
            shader.setValue(TextFormat(lightStr, "type"), static_cast<int>(light->type));

            if (config.shader.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
                shader.setValue(TextFormat(lightStr, "shadow"), light->shadow);
                if (light->shadow) {
                    shader.setValue(TextFormat(lightStr, "shadowBias"), light->shadowBias);
                }
            }

            switch (light->type) {
                case R3D_DIRLIGHT:
                    shader.setValue(TextFormat(lightStr, "direction"), light->direction);
                    if (light->shadow && (config.shader.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW)) {
                        shader.setValue(TextFormat("uMatLightMVP[%i]", lightIndex), light->vpMatrix());
                        shader.setValue(TextFormat(lightStr, "shadowMapTxlSz"), light->map->texelWidth());
                        shader.bindTexture(TextFormat(lightStr, "shadowMap"),
                            light->map->attachement(GLAttachement::DEPTH));
                    }
                    break;
                case R3D_SPOTLIGHT:
                    shader.setValue(TextFormat(lightStr, "position"), light->position);
                    shader.setValue(TextFormat(lightStr, "direction"), light->direction);
                    shader.setValue(TextFormat(lightStr, "maxDistance"), light->maxDistance);
                    shader.setValue(TextFormat(lightStr, "attenuation"), light->attenuation);
                    shader.setValue(TextFormat(lightStr, "innerCutOff"), light->innerCutOff);
                    shader.setValue(TextFormat(lightStr, "outerCutOff"), light->outerCutOff);
                    if (light->shadow && (config.shader.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW)) {
                        shader.setValue(TextFormat("uMatLightMVP[%i]", lightIndex), light->vpMatrix());
                        shader.setValue(TextFormat(lightStr, "shadowMapTxlSz"), light->map->texelWidth());
                        shader.bindTexture(TextFormat(lightStr, "shadowMap"),
                            light->map->attachement(GLAttachement::DEPTH));
                    }
                    break;
                case R3D_OMNILIGHT:
                    shader.setValue(TextFormat(lightStr, "position"), light->position);
                    shader.setValue(TextFormat(lightStr, "maxDistance"), light->maxDistance);
                    shader.setValue(TextFormat(lightStr, "attenuation"), light->attenuation);
                    if (light->shadow && (config.shader.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW)) {
                        shader.bindTexture(TextFormat(lightStr, "shadowCubemap"),
                            light->map->attachement(GLAttachement::COLOR_0));
                    }
                    break;
            }

            lightIndex++;
        }

        for (int i = lightIndex; i < SHADER_LIGHT_COUNT; i++) {
            shader.setValue(TextFormat("uLights[%i].enabled", i), false);
        }
    }

    // Try binding vertex array objects (VAO) or use VBOs if not possible
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION]);
        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD]);
        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);

        // Bind mesh VBO data: vertex normals (shader-location = 2)
        if (config.shader.diffuse != R3D_DIFFUSE_UNSHADED) {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL]);
            rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3)
        if (config.shader.flags & R3D_MATERIAL_FLAG_VERTEX_COLOR) {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR]);
            rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR, 4, RL_UNSIGNED_BYTE, 1, 0, 0);
            rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR);
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4)
        if (config.shader.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
            rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT]);
            rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5)
        //if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        //{
        //    rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2]);
        //    rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
        //    rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        //}

        // Bind mesh VBO data: vertex bone ids (shader-location = 6)
        //if (material.shader.locs[SHADER_LOC_VERTEX_BONEIDS] != -1)
        //{
        //    rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS]);
        //    rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEIDS], 4, RL_UNSIGNED_BYTE, 0, 0, 0);
        //    rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEIDS]);
        //}

        // Bind mesh VBO data: vertex bone weights (shader-location = 7)
        //if (material.shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS] != -1)
        //{
        //    rlEnableVertexBuffer(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS]);
        //    rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS], 4, RL_FLOAT, 0, 0, 0);
        //    rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS]);
        //}

        if (mesh.indices != NULL) {
            rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
        }
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) {
            matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        } else {
            // Setup current eye viewport (half screen width)
            glViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        shader.setValue("uMatMVP", matModelViewProjection);

        // Draw mesh
        if (mesh.indices != NULL) {
            rlDrawVertexArrayElements(0, 3 * mesh.triangleCount, 0);
        } else {
            rlDrawVertexArray(0, mesh.vertexCount);
        }
    }

    // Unbind all bound texture maps
    shader.unbindTextures();

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
}

inline void Renderer::renderShadowMapDirectional(const Light& light, const std::vector<DrawCallShadow>& batch) const
{
    mShaderDepth.use();
    light.map->begin();
    {
        glClear(GL_DEPTH_BUFFER_BIT);

        rlMatrixMode(RL_PROJECTION);
        rlPushMatrix();

        rlSetMatrixProjection(light.projMatrix());
        rlSetMatrixModelview(light.viewMatrix());

        for (const auto& drawCall : batch) {
            drawSurfaceShadow(drawCall, light);
        }

        rlMatrixMode(RL_PROJECTION);
        rlPopMatrix();

        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();
    }
    light.map->end();
}

inline void Renderer::renderShadowMapSpot(const Light& light, const std::vector<DrawCallShadow>& batch) const
{
    mShaderDepth.use();
    light.map->begin();
    {
        glClear(GL_DEPTH_BUFFER_BIT);

        rlMatrixMode(RL_PROJECTION);
        rlPushMatrix();

        rlSetMatrixProjection(light.projMatrix());
        rlSetMatrixModelview(light.viewMatrix());

        for (const auto& drawCall : batch) {
            drawSurfaceShadow(drawCall, light);
        }

        rlMatrixMode(RL_PROJECTION);
        rlPopMatrix();

        rlMatrixMode(RL_MODELVIEW);
        rlLoadIdentity();
    }
    light.map->end();
}

inline void Renderer::renderShadowMapOmni(const Light& light, const std::vector<DrawCallShadow>& batch) const
{
    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlSetMatrixProjection(light.projMatrix());

    mShaderDepthCube.use();
    light.map->begin();
    {
        for (int i = 0; i < 6; i++) {
            light.map->bindFace(GLAttachement::COLOR_0, i);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            rlSetMatrixModelview(light.viewMatrix(i));
            for (const auto& drawCall : batch) {
                drawSurfaceShadow(drawCall, light);
            }
        }
    }
    light.map->end();

    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

} // namespace r3d

#endif // R3D_DETAIL_RENDERER_HPP
