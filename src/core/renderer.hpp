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

#ifndef R3D_RENDERER_HPP
#define R3D_RENDERER_HPP

#include "r3d.h"

#include "../detail/GL/GLFramebuffer.hpp"
#include "../detail/RL/RLCamera3D.hpp"
#include "../detail/RL/RLTexture.hpp"
#include "../detail/RL/RLShader.hpp"
#include "../detail/GL/GLShader.hpp"

#include "../detail/ShaderMaterial.hpp"
#include "../detail/BloomRenderer.hpp"
#include "../detail/RenderTarget.hpp"
#include "../detail/ShaderCode.hpp"
#include "../detail/BatchMap.hpp"
#include "../detail/Frustum.hpp"
#include "../detail/IDMan.hpp"
#include "../detail/Quad.hpp"
#include "../detail/GL.hpp"

#include "../objects/skybox.hpp"
#include "../objects/model.hpp"
#include "./lighting.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <variant>
#include <cstdio>
#include <memory>
#include <map>

/* Declaration of global Renderer instance */

namespace r3d { class Renderer; }
extern std::unique_ptr<r3d::Renderer> gRenderer;

/* Renderer implementation */

namespace r3d {

/**
 * @brief Stores a draw call for rendering in shadow maps.
 */
class DrawCall_Shadow
{
public:
    /**
     * @struct Surface
     * @brief Structure representing a surface to be rendered.
     */
    struct Surface {
        const Mesh *mesh;           ///< Pointer to the surface mesh.
        const Matrix transform;     ///< Transformation matrix for the mesh.
    };

    /**
     * @struct Sprite
     * @brief Structure representing a sprite to be rendered.
     */
    struct Sprite {
        const R3D_Sprite *sprite;   ///< Pointer to the sprite.
        const Matrix transform;     ///< Transformation matrix for the sprite.
    };

    /**
     * @struct ParticlesCPU
     * @brief Structure representing a particle system to be rendered.
     */
    struct ParticlesCPU {
        const R3D_ParticleSystemCPU *system; ///< Pointer to the particle system.
    };

public:
    /**
     * @brief Constructs a draw call for a mesh.
     */
    DrawCall_Shadow(const Mesh* mesh, const Matrix& transform);

    /**
     * @brief Constructs a draw call for a sprite.
     */
    DrawCall_Shadow(const R3D_Sprite* sprite, const Matrix& transform);

    /**
     * @brief Constructs a draw call for a particle system.
     */
    DrawCall_Shadow(const R3D_ParticleSystemCPU* system);

    /**
     * @brief Draws the object (mesh, sprite, or particle system) for shadow mapping.
     */
    void draw(const Light& light) const;

private:
    /**
     * @brief Draws the mesh for shadow mapping.
     */
    void drawMesh(const Light& light) const;

    /**
     * @brief Draws the sprite for shadow mapping.
     */
    void drawSprite(const Light& light) const;

    /**
     * @brief Draws the particle system for shadow mapping.
     */
    void drawParticlesCPU(const Light& light) const;

private:
    std::variant<Surface, Sprite, ParticlesCPU> mCall; ///< Holds either a surface, sprite, or particle system.
};

/**
 * @brief Stores a draw call for rendering in the main scene.
 */
class DrawCall_Scene
{
public:
    /**
     * @struct Surface
     * @brief Structure representing a surface to be rendered in the scene.
     */
    struct Surface {
        struct {
            const Mesh *mesh;           ///< Pointer to the mesh to be rendered.
            R3D_Material material;      ///< Material copy for rendering with different parameters.
        } surface;                      ///< Surface information for the draw call.
        ShaderLightArray lights;        ///< Array of light pointers influencing this draw call.
        Matrix transform;               ///< Transformation matrix for the draw call.
    };

    /**
     * @struct Sprite
     * @brief Structure representing a sprite to be rendered in the scene.
     */
    struct Sprite {
        const R3D_Sprite *sprite;       ///< Pointer to the sprite to be rendered.
        ShaderLightArray lights;        ///< Array of light pointers influencing the sprite.
        Matrix transform;               ///< Transformation matrix for the sprite.
    };

    /**
     * @struct ParticlesCPU
     * @brief Structure representing a particle system to be rendered in the scene.
     */
    struct ParticlesCPU {
        const R3D_ParticleSystemCPU *system;    ///< Pointer to the particle system to be rendered.
        ShaderLightArray lights;                ///< Array of light pointers influencing the particle system.
    };

public:
    /**
     * @brief Constructs a draw call for a surface to be rendered in the scene.
     */
    DrawCall_Scene(const R3D_Surface& surface, const Matrix& transform, const ShaderLightArray& lights);

    /**
     * @brief Constructs a draw call for a sprite to be rendered in the scene.
     */
    DrawCall_Scene(const R3D_Sprite* sprite, const Matrix& transform, const ShaderLightArray& lights);

    /**
     * @brief Constructs a draw call for a particle system to be rendered in the scene.
     */
    DrawCall_Scene(const R3D_ParticleSystemCPU* system, const ShaderLightArray& lights);

    /**
     * @brief Executes the draw call using the provided shader material.
     */
    void draw(ShaderMaterial& shader) const;

    /**
     * @brief Retrieves the transformation matrix of the surface, if applicable.
     * @return Returns `nullptr` if the underlying object does not have a direct transformation.
     */
    const Matrix* getTransform() const;

private:
    /**
     * @brief Draws the mesh for this draw call using the shader material.
     */
    void drawMesh(ShaderMaterial& shader) const;

    /**
     * @brief Draws the sprite for this draw call using the shader material.
     */
    void drawSprite(ShaderMaterial& shader) const;

    /**
     * @brief Draws the particle system for this draw call using the shader material.
     */
    void drawParticlesCPU(ShaderMaterial& shader) const;

private:
    std::variant<Surface, Sprite, ParticlesCPU> mCall; ///< Holds either a surface (mesh), sprite, or particle system for rendering in the scene.
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
    friend class DrawCall_Shadow;
    friend class DrawCall_Scene;

public:
    R3D_Environment environment;                ///< Environment settings for the renderer.
    const RenderTexture *customRenderTarget;    ///< Custom raylib render target to which the blit is performed instead of the main framebuffer.
    R3D_DepthSortingOrder depthSortingOrder;    ///< Specifies the deoth sorting order of surfaces before rendering.
    float shadowsUpdateFrequency;               ///< Reciprocal of the number of shadow map updates per second.  
    float shadowsUpdateTimer;                   ///< Timer used to control the frequency of shadow map updates.
    int activeLayers;                           ///< `R3D_Layer` that are active.

    int flags;  /**< Copies of the flags assigned during initialization,
                 *   may be modified during execution, except for flags that
                 *   require specific data initialization, such as debug flags.
                 */

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
     * @brief Computes the global transformation matrix for an object.
     * @return The computed global transformation matrix.
     */
    Matrix getGlobalTrasformMatrix(R3D_BillboardMode billboard, const R3D_Transform& transform,
                                   const Vector3& position, const Vector3& rotationAxis,
                                   float rotationAngle, const Vector3& scale);

    /**
     * @brief Determines if an object is visible within the current view frustum.
     * @return true if the object is visible; false otherwise.
     */
    template <typename Object>
    bool isObjectVisible(const Object& object, const BoundingBox& globalAABB);

    /**
     * @brief Prepares lighting and shadow mapping data for a given object.
     */
    template <typename Object>
    void setupLightsAndShadows(const Object& object, const BoundingBox& globalAABB,
                               const Matrix& globalTransform, ShaderLightArray* lightArray);

    /**
     * @brief Adds an object and its associated lighting data to the rendering batch.
     */
    template <typename Object>
    void addObjectToSceneBatch(const Object& object, const Matrix& globalTransform, const ShaderLightArray& lightArray);

    /**
     * @brief Executes the shadow map rendering pass.
     */
    void renderShadowPass();

    /**
     * @brief Executes the main scene rendering pass.
     */
    void renderScenePass();

    /**
     * @brief Executes the post-processing pass for the rendered scene.
     */
    void renderPostProcessPass();

    /**
     * @brief Presents the final render (scene + post-process) to the target framebuffer.
     * 
     * If no framebuffer is defined, the default framebuffer is used.
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
     * This function is used to render a mesh for the shadow map during the shadow mapping pass. It takes the light source 
     * casting shadows, the mesh to be rendered, and its transformation matrix as inputs to compute the correct shadow rendering.
     * 
     * @param light The light casting shadows on the scene. This determines the direction and type of shadow.
     * @param mesh The mesh to be rendered for shadow mapping. It contains the geometry of the object.
     * @param transform The transformation matrix applied to the mesh, including its position, rotation, and scale in the world.
     */
    void drawMeshShadow(const Light& light, const Mesh& mesh, const Matrix& transform) const;

    /**
     * @brief Draws a surface in the main scene render pass.
     * 
     * This function is used to render a mesh in the main scene, with materials and shaders applied. It takes the mesh, its 
     * transformation matrix, the shader material to use for rendering, and the material configuration to properly render the 
     * object within the scene.
     * 
     * @param mesh The mesh to be rendered in the scene. It contains the object's geometry data.
     * @param transform The transformation matrix applied to the mesh, which controls its position, rotation, and scale.
     * @param shader The shader material used for rendering the surface. It controls the appearance of the mesh.
     * @param config The material configuration settings that specify how the mesh should be rendered (e.g., material properties).
     */
    void drawMeshScene(const Mesh& mesh, const Matrix& transform, ShaderMaterial& shader, R3D_MaterialConfig config) const;

private:
    int mInternalWidth;                         ///< Internal framebuffer width.
    int mInternalHeight;                        ///< Internal framebuffer height.
    RenderTarget mTargetScene;                  ///< Render target for the main scene.
    RenderTarget mTargetPostFX;                 ///< Render target for post-processing effects.
    BloomRenderer mBloomRenderer;               ///< Blur renderer used for the bloom effect.

    std::unordered_map<
        R3D_MaterialShaderConfig, ShaderMaterial,
        MaterialShaderConfigHash, MaterialShaderConfigEqual
    > mShaderMaterials; ///< Shader map for material properties.

    BatchMap<R3D_MaterialConfig, DrawCall_Scene> mSceneBatches;      ///< Scene draw calls sorted by material.
    BatchMap<R3D_Light, DrawCall_Shadow> mShadowBatches;             ///< Shadow draw calls for each light.

    std::map<R3D_Light, Light> mLights;         ///< Map of lights and their data.
    R3D_MaterialConfig mDefaultMaterialConfig;  ///< Default material configuration.
    IDMan<R3D_Light> mLightIDMan;               ///< Light ID manager.

    RLTexture mBlackTexture2D;      ///< Black placeholder texture.
    RLTexture mWhiteTexture2D;      ///< White placeholder texture.
    Quad mQuad;                     ///< Quad used for rendering.

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
    , customRenderTarget(nullptr)
    , depthSortingOrder(R3D_DEPTH_SORT_DISABLED)
    , shadowsUpdateFrequency(1.0f / 30)
    , shadowsUpdateTimer(1.0f / 30)
    , activeLayers(R3D_LAYER_1)
    , flags(flags)
    , mTargetScene(mInternalWidth, mInternalHeight)
    , mTargetPostFX(mInternalWidth, mInternalHeight)
    , mBloomRenderer(mInternalWidth, mInternalHeight)
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
    // DEPTH: Contains the depth of the scene...
    // COLOR_0: Contains the final colors of the scene
    // COLOR_1: Contains the colors of areas brighter than the HDR threshold for bloom, the Alpha component contains the perceived luminance

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
}

inline void Renderer::loadMaterialConfig(R3D_MaterialConfig config)
{
    // Instantiates an entity vector in 'm_material_batches' for this material type if it has not already been created

    if (!mSceneBatches.isBatchExist(config)) {
        mSceneBatches.addBatch(config);
    }

    // Compiles a shader for the given configuration if necessary

    const auto it_material_shader = mShaderMaterials.find(config.shader);

    if (it_material_shader != mShaderMaterials.cend()) {
        return;
    }

    mShaderMaterials.emplace(config.shader, config.shader);
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
    if (flags & R3D_FLAG_ASPECT_KEEP) {
        aspect = static_cast<float>(mInternalWidth) / mInternalHeight;
    } else {
        aspect = static_cast<float>(GetScreenWidth()) / GetScreenHeight();
    }

    // Retrieves the camera's view and projection matrices

    mMatCameraView = mCamera.viewMatrix();
    mMatCameraProj = mCamera.projMatrix(aspect);

    // Computes the camera's frustum if necessary

    if (!(flags & R3D_FLAG_NO_FRUSTUM_CULLING)) {
        mFrustumCamera = Frustum(MatrixMultiply(mMatCameraView, mMatCameraProj));
    }
}

inline Matrix Renderer::getGlobalTrasformMatrix(R3D_BillboardMode billboard, const R3D_Transform& transform, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector3& scale)
{
    Matrix mat = MatrixMultiply(
        MatrixMultiply(
            MatrixScale(scale.x, scale.y, scale.z),
            MatrixRotate(rotationAxis, rotationAngle * DEG2RAD)
        ),
        MatrixTranslate(position.x, position.y, position.z)
    );

    mat = MatrixMultiply(mat, R3D_TransformToGlobal(&transform));
    mat = MatrixMultiply(mat, rlGetMatrixTransform());

    if (billboard != R3D_BILLBOARD_DISABLED) {
        const Vector3 translation = getMatrixTrasnlation(mat);
        mat = MatrixMultiply(mat, getBillboardRotationMatrix(
            billboard, translation, mCamera.position
        ));
    }

    return mat;
}

template <typename Object>
inline bool Renderer::isObjectVisible(const Object& object, const BoundingBox& globalAABB)
{
    if (object.shadow == R3D_CAST_SHADOW_ONLY) return false;
    if (flags & R3D_FLAG_NO_FRUSTUM_CULLING) return true;
    if (!(activeLayers & object.layer)) return false;
    return mFrustumCamera.aabbIn(globalAABB);
}

template <typename Object>
inline void Renderer::setupLightsAndShadows(const Object& object, const BoundingBox& globalAABB, const Matrix& globalTransform, ShaderLightArray* lightArray)
{
    if (!(activeLayers & object.layer)) {   //< If the object's layer is inactive, return
        return;
    }

    bool shadow = (object.shadow != R3D_CAST_OFF)
        && (shadowsUpdateTimer >= shadowsUpdateFrequency);

    if (!shadow && lightArray == nullptr) {
        return;
    }

    int lightCount = 0;

    for (const auto& [id, light] : mLights) {

        if (!light.enabled || (!light.shadow && lightArray == nullptr)) continue;

        if (!(activeLayers & light.layers)) continue;   //< If none of the light's layers are active, continue
        if (!(light.layers & object.layer)) continue;   //< If the light does not affect the object's layer, continue

        // Here, we use a rather naive approach. If the light is not directional—
        // for instance, it does not simulate the sun—then we check whether the squared 
        // distance between the origin of the model and the light exceeds the maximum range 
        // of the light's effective field. If it does, we can safely omit this light source for the model.

        // Using the squared distance is faster to compute and provides a certain margin, 
        // making it acceptable in this context.

        const float lightMaxDistSqr = light.maxDistance * light.maxDistance;
        const Vector3 globalPos = getMatrixTrasnlation(globalTransform);

        if (light.type != R3D_DIRLIGHT && Vector3DistanceSqr(globalPos, light.position) > lightMaxDistSqr) {
            continue;
        }

        // Here, if the light is not an omnilight, we perform a frustum test
        // from its point of view. If the object is not "visible" from the light, we skip it

        if (light.type != R3D_OMNILIGHT && !light.frustum.aabbIn(globalAABB)) {
            continue;
        }

        // Here, if the light casts shadows, we add the object to its set of objects for rendering in its shadow map

        if (shadow && light.shadow) {
            if constexpr (std::is_same_v<Object, R3D_Model>) {
                for (const auto& surface : static_cast<Model*>(object.internal)->surfaces) {
                    mShadowBatches.pushDrawCall(id, DrawCall_Shadow(&surface.mesh, globalTransform));
                }
            } else if constexpr (std::is_same_v<Object, R3D_Sprite>) {
                mShadowBatches.pushDrawCall(id, DrawCall_Shadow(&object, globalTransform));
            } else if constexpr (std::is_same_v<Object, R3D_ParticleSystemCPU>) {
                mShadowBatches.pushDrawCall(id, DrawCall_Shadow(&object));
            }
        }

        // Here, if a light array has been given and it is not full, we add this light to the array

        if (lightArray && lightCount < SHADER_LIGHT_COUNT) {
            (*lightArray)[lightCount++] = &light;
        }
    }
}

template <typename Object>
inline void Renderer::addObjectToSceneBatch(const Object& object, const Matrix& globalTransform, const ShaderLightArray& lightArray)
{
    if constexpr (std::is_same_v<Object, R3D_Model>) {
        for (const auto& surface : static_cast<Model*>(object.internal)->surfaces) {
            mSceneBatches.pushDrawCall(surface.material.config,
                DrawCall_Scene(surface, globalTransform, lightArray)
            );
        }
    } else if constexpr (std::is_same_v<Object, R3D_Sprite>) {
        mSceneBatches.pushDrawCall(object.material.config,
            DrawCall_Scene(&object, globalTransform, lightArray)
        );
    } else if constexpr (std::is_same_v<Object, R3D_ParticleSystemCPU>) {
        mSceneBatches.pushDrawCall(object.surface.material.config,
            DrawCall_Scene(&object, lightArray)
        );
    }
}

inline void Renderer::renderShadowPass()
{
    rlDisableColorBlend();  /**< We deactivate the color bleding because the omni
                             *   lights write the distances in a color attachment
                             */

    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();

    for (auto& [lightID, batch] : mShadowBatches) {
        if (batch.empty()) continue;

        const auto& light = mLights.at(lightID);

        rlSetMatrixProjection(light.projMatrix());

        switch (light.type) {
            case R3D_DIRLIGHT:
            case R3D_SPOTLIGHT: {
                mShaderDepth.use();
                light.map->begin();
                {
                    glClear(GL_DEPTH_BUFFER_BIT);
                    rlSetMatrixModelview(light.viewMatrix());
                    for (const auto& drawCall : batch) {
                        drawCall.draw(light);
                    }
                }
                light.map->end();
            } break;
            case R3D_OMNILIGHT: {
                mShaderDepthCube.use();
                light.map->begin();
                {
                    for (int i = 0; i < 6; i++) {
                        light.map->bindFace(GLAttachement::COLOR_0, i);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        rlSetMatrixModelview(light.viewMatrix(i));
                        for (const auto& drawCall : batch) {
                            drawCall.draw(light);
                        }
                    }
                }
                light.map->end();
            } break;
        }

        batch.clear();
    }

    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

inline void Renderer::renderScenePass()
{
    const Vector3 camPos = mCamera.position;

    switch (depthSortingOrder) {
        case R3D_DEPTH_SORT_FAR_TO_NEAR: {
            for (auto& [_, batch] : mSceneBatches) {
                std::sort(batch.begin(), batch.end(), [camPos](const DrawCall_Scene& a, const DrawCall_Scene& b) {
                    if (a.getTransform() && b.getTransform()) {
                        float distanceA = Vector3DistanceSqr(camPos, getMatrixTrasnlation(*a.getTransform()));
                        float distanceB = Vector3DistanceSqr(camPos, getMatrixTrasnlation(*b.getTransform()));
                        return distanceA > distanceB;
                    }
                    return a.getTransform() != nullptr;
                });
            }
        } break;
        case R3D_DEPTH_SORT_NEAR_TO_FAR: {
            for (auto& [_, batch] : mSceneBatches) {
                std::sort(batch.begin(), batch.end(), [camPos](const DrawCall_Scene& a, const DrawCall_Scene& b) {
                    if (a.getTransform() && b.getTransform()) {
                        float distanceA = Vector3DistanceSqr(camPos, getMatrixTrasnlation(*a.getTransform()));
                        float distanceB = Vector3DistanceSqr(camPos, getMatrixTrasnlation(*b.getTransform()));
                        return distanceA < distanceB;
                    }
                    return a.getTransform() != nullptr;
                });
            }
        } break;
        default:
            break;
    }

    mTargetScene.begin();
    {
        const R3D_Skybox *skybox = environment.world.skybox;

        /* Setup view port and clear target scene framebuffer */

        if (flags & R3D_FLAG_ASPECT_KEEP) {
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

        if (skybox != nullptr) {
            Vector3 rotSkybox = Vector3Scale(skybox->rotation, DEG2RAD);
            Quaternion quatSkybox = QuaternionFromEuler(rotSkybox.x, rotSkybox.y, rotSkybox.z);
            static_cast<Skybox*>(skybox->internal)->draw(quatSkybox);
        }

        /* Preparing the scene rendering */

        glDisablei(GL_BLEND, 1);    /*< Here we disable color blending for the output `COLOR_1`
                                                   *  which corresponds to the HDR values for bloom calculation, as
                                                   *  we store the perceived luminance in the alpha component.
                                                   */

        /* Render surfaces */

        for (auto& [config, batch] : mSceneBatches) {
            if (batch.empty()) continue;

            // TODO: Find a method to reduce calls to state changes, even if probably ignored by most drivers...

            if (config.blendMode == R3D_BLEND_DISABLED) {
                glDisablei(GL_BLEND, 0);
            } else {
                glEnablei(GL_BLEND, 0);
                rlSetBlendMode(config.blendMode - 1);
            }

            if (config.cullMode == R3D_CULL_DISABLED) {
                rlDisableBackfaceCulling();
            } else {
                rlEnableBackfaceCulling();
                rlSetCullFace(config.cullMode - 1);
            }

            ShaderMaterial& shader = mShaderMaterials.at(config.shader);

            shader.begin();
            {
                shader.setEnvironment(environment, mCamera.position);
                for (const auto& drawCall : batch) {
                    drawCall.draw(shader);
                }
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
}

inline void Renderer::renderPostProcessPass()
{
    /* Apply gaussian blur (two passes) for bloom if needed */

    if (environment.bloom.mode != R3D_BLOOM_DISABLED) {
        mBloomRenderer.render(
            mTargetScene.attachement(GLAttachement::COLOR_1),
            environment.bloom.iterations
        );
    }

    /* Apply post effects */

    mTargetPostFX.begin();
        mShaderPostFX.begin();
        {
            mShaderPostFX.setValue("uBloomMode", static_cast<int>(environment.bloom.mode));

            if (environment.bloom.mode != R3D_BLOOM_DISABLED) {
                mShaderPostFX.bindTexture("uTexBloomBlurHDR", mBloomRenderer.result());
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
}

inline void Renderer::present()
{
    bool blitLinear = R3D_FLAG_BLIT_LINEAR;
    GLint target = customRenderTarget ? customRenderTarget->id : 0;

    if (flags & R3D_FLAG_ASPECT_KEEP) {
        mTargetPostFX.blitAspectKeep(target, GLAttachement::COLOR_0, false, blitLinear);
        mTargetScene.blitAspectKeep(target, GLAttachement::NONE, true, false);
    } else {
        mTargetPostFX.blitAspectExpand(target, GLAttachement::COLOR_0, false, blitLinear);
        mTargetScene.blitAspectExpand(target, GLAttachement::NONE, true, false);
    }
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
    mBloomRenderer.resize(newWidth, newHeight);
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

inline void Renderer::drawMeshShadow(const Light& light, const Mesh& mesh, const Matrix& transform) const
{
    if (!rlEnableVertexArray(mesh.vaoId)) {
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
        if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] > 0) {
            rlEnableVertexBufferElement(mesh.vboId[6]);
        }
    }

    ::Matrix matMVP = MatrixMultiply(
        MatrixMultiply(transform, rlGetMatrixModelview()),
        rlGetMatrixProjection()
    );

    if (light.type == R3D_OMNILIGHT) {
        rlSetUniform(mShaderDepthCube.locs[SHADER_LOC_VECTOR_VIEW], &light.position, SHADER_UNIFORM_VEC3, 1);
        rlSetUniformMatrix(mShaderDepthCube.locs[SHADER_LOC_MATRIX_MODEL], transform);
        rlSetUniformMatrix(mShaderDepthCube.locs[SHADER_LOC_MATRIX_MVP], matMVP);
    } else {
        rlSetUniformMatrix(mShaderDepth.locs[SHADER_LOC_MATRIX_MVP], matMVP);
    }

    if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] == 0) {
        rlDrawVertexArray(0, mesh.vertexCount);
    } else {
        rlDrawVertexArrayElements(0, 3 * mesh.triangleCount, 0);
    }

    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();
}

inline void Renderer::drawMeshScene(const Mesh& mesh, const Matrix& transform, ShaderMaterial& shader, R3D_MaterialConfig config) const
{
    Matrix matView = rlGetMatrixModelview();

    Matrix matModelView = MatrixMultiply(transform, matView);
    Matrix matProjection = rlGetMatrixProjection();

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

        if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] > 0) {
            rlEnableVertexBufferElement(mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES]);
        }
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++) {
        if (eyeCount == 1) {
            shader.setMatMVP(MatrixMultiply(matModelView, matProjection));
        } else {
            glViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            shader.setMatMVP(MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye)));
        }
        if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] == 0) {
            rlDrawVertexArray(0, mesh.vertexCount);
        } else {
            rlDrawVertexArrayElements(0, 3 * mesh.triangleCount, 0);
        }
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
}


/* DrawCall_Shadow implementation */

inline DrawCall_Shadow::DrawCall_Shadow(const Mesh* mesh, const Matrix& transform)
    : mCall(Surface { mesh, transform })
{ }

inline DrawCall_Shadow::DrawCall_Shadow(const R3D_Sprite* sprite, const Matrix& transform)
    : mCall(Sprite { sprite, transform })
{ }

inline DrawCall_Shadow::DrawCall_Shadow(const R3D_ParticleSystemCPU* system)
    : mCall(ParticlesCPU { system })
{ }

inline void DrawCall_Shadow::draw(const Light& light) const
{
    switch (mCall.index()) {
        case 0: drawMesh(light); break;
        case 1: drawSprite(light); break;
        case 2: drawParticlesCPU(light); break;
    }
}

inline void DrawCall_Shadow::drawMesh(const Light& light) const
{
    const auto& call = std::get<0>(mCall);
    gRenderer->drawMeshShadow(light, *call.mesh, call.transform);
}

inline void DrawCall_Shadow::drawSprite(const Light& light) const
{
    const auto& call = std::get<1>(mCall);

    unsigned int vbo[9]{};
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION] = gRenderer->mQuad.vbo();
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD] = gRenderer->mQuad.vbo();
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL] = gRenderer->mQuad.vbo();
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] = gRenderer->mQuad.ebo();

    Mesh mesh {
        .triangleCount = 2,
        .vaoId = gRenderer->mQuad.vao(),
        .vboId = vbo
    };

    gRenderer->drawMeshShadow(light, mesh, call.transform);
}

inline void DrawCall_Shadow::drawParticlesCPU(const Light& light) const
{
    const auto& call = std::get<2>(mCall);
    for (int i = 0; i < call.system->particleCount; i++) {
        const R3D_Particle& particle = call.system->particles[i];
        Matrix transform = MatrixMultiply(
            MatrixMultiply(
                MatrixScale(particle.scale.x, particle.scale.y, particle.scale.z),
                MatrixRotateXYZ(particle.rotation)
            ),
            MatrixTranslate(particle.position.x, particle.position.y, particle.position.z)
        );
        gRenderer->drawMeshShadow(
            light, call.system->surface.mesh,
            MatrixMultiply(transform, rlGetMatrixTransform())
        );
    }
}


/* DrawCall_Scene implementation */

inline DrawCall_Scene::DrawCall_Scene(const R3D_Surface& surface, const Matrix& transform, const ShaderLightArray& lights)
    : mCall(Surface { &surface.mesh, surface.material, lights, transform })
{ }

inline DrawCall_Scene::DrawCall_Scene(const R3D_Sprite* sprite, const Matrix& transform, const ShaderLightArray& lights)
    : mCall(Sprite { sprite, lights, transform })
{ }

inline DrawCall_Scene::DrawCall_Scene(const R3D_ParticleSystemCPU* system, const ShaderLightArray& lights)
    : mCall(ParticlesCPU { system, lights })
{ }

inline void DrawCall_Scene::draw(ShaderMaterial& shader) const
{
    switch (mCall.index()) {
        case 0: drawMesh(shader); break;
        case 1: drawSprite(shader); break;
        case 2: drawParticlesCPU(shader); break;
    }
}

inline const Matrix* DrawCall_Scene::getTransform() const {
    switch (mCall.index()) {
        case 0: return &std::get<0>(mCall).transform;
        case 1: return &std::get<1>(mCall).transform;
        default: break;
    }
    return nullptr;
}

inline void DrawCall_Scene::drawMesh(ShaderMaterial& shader) const
{
    const auto& call = std::get<0>(mCall);

    shader.setMaterial(call.surface.material);
    shader.setMatModel(call.transform);
    shader.setLights(call.lights);

    gRenderer->drawMeshScene(*call.surface.mesh, call.transform, shader, call.surface.material.config);
}

inline void DrawCall_Scene::drawSprite(ShaderMaterial& shader) const
{
    const auto& call = std::get<1>(mCall);

    shader.setMaterial(call.sprite->material);
    shader.setMatModel(call.transform);
    shader.setLights(call.lights);

    unsigned int vbo[9]{};
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION] = gRenderer->mQuad.vbo();
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD] = gRenderer->mQuad.vbo();
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL] = gRenderer->mQuad.vbo();
    vbo[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] = gRenderer->mQuad.ebo();

    Mesh mesh {
        .triangleCount = 2,
        .vaoId = gRenderer->mQuad.vao(),
        .vboId = vbo
    };

    gRenderer->drawMeshScene(mesh, call.transform, shader, call.sprite->material.config);
}

inline void DrawCall_Scene::drawParticlesCPU(ShaderMaterial& shader) const
{
    auto& call = std::get<2>(mCall);

    Color baseColor = call.system->surface.material.albedo.color;
    const Vector3& camPos = gRenderer->mCamera.position;

    std::sort(call.system->particles, call.system->particles + call.system->particleCount,
        [camPos](const R3D_Particle& a, const R3D_Particle& b) {
            float distanceA = Vector3DistanceSqr(camPos, a.position);
            float distanceB = Vector3DistanceSqr(camPos, b.position);
            return distanceA > distanceB;
        }
    );

    for (int i = 0; i < call.system->particleCount; i++) {
        const R3D_Particle& particle = call.system->particles[i];
        Matrix transform = MatrixMultiply(
            MatrixMultiply(
                MatrixScale(particle.scale.x, particle.scale.y, particle.scale.z),
                MatrixRotateXYZ(particle.rotation)
            ),
            MatrixTranslate(particle.position.x, particle.position.y, particle.position.z)
        );

        if (call.system->billboard != R3D_BILLBOARD_DISABLED) {
            Vector3 modelPos = getMatrixTrasnlation(transform);
            Matrix billboardRotation = getBillboardRotationMatrix(
                call.system->billboard, modelPos, gRenderer->mCamera.position
            );
            transform = MatrixMultiply(transform, billboardRotation);
        }

        R3D_Material material = call.system->surface.material;

        material.albedo.color = (Color) {
            static_cast<uint8_t>((baseColor.r * particle.color.r) / 255),
            static_cast<uint8_t>((baseColor.g * particle.color.g) / 255),
            static_cast<uint8_t>((baseColor.b * particle.color.b) / 255),
            static_cast<uint8_t>((baseColor.a * particle.color.a) / 255)
        };

        shader.setMaterial(material);
        shader.setMatModel(transform);
        shader.setLights(call.lights);

        gRenderer->drawMeshScene(call.system->surface.mesh, transform, shader, material.config);
    }
}

} // namespace r3d

#endif // R3D_RENDERER_HPP
