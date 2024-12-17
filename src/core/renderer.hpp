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

#include "detail/ShaderCodes.hpp"

#include "../detail/GL/GLFramebuffer.hpp"
#include "../detail/RL/RLCamera3D.hpp"
#include "../detail/RL/RLTexture.hpp"
#include "../detail/RL/RLShader.hpp"

#include "../detail/GL/GLTexture.hpp"
#include "../detail/GL/GLShader.hpp"

#include "../detail/ShaderMaterial.hpp"
#include "../detail/RenderTarget.hpp"
#include "../detail/BatchMap.hpp"
#include "../detail/Frustum.hpp"
#include "../detail/IDMan.hpp"
#include "../detail/Quad.hpp"
#include "../detail/GL.hpp"

#include "../objects/skybox.hpp"
#include "../objects/model.hpp"
#include "./lighting.hpp"

#include <algorithm>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <unordered_map>
#include <cstdint>
#include <variant>
#include <vector>
#include <cstdio>
#include <cfloat>
#include <memory>
#include <map>

/* Declaration of global Renderer instance */

namespace r3d { class Renderer; }
extern std::unique_ptr<r3d::Renderer> gRenderer;

/* Renderer implementation */

namespace r3d {

/**
 * @brief Stores a draw call for rendering in shadow maps.
 * 
 * This class encapsulates a draw call, which can either be a mesh, sprite, or particle system, for rendering in shadow maps.
 * The draw call is evaluated and executed to render the object in the shadow map from the perspective of a light source.
 */
class DrawCall_Shadow
{
public:
    /**
     * @struct Surface
     * @brief Structure representing a surface to be rendered.
     * 
     * This structure contains the mesh and its associated transformation matrix for rendering.
     */
    struct Surface {
        const Mesh *mesh;           ///< Pointer to the surface mesh to be rendered.
        const Matrix transform;     ///< Transformation matrix for the draw call.
    };

    /**
     * @struct Sprite
     * @brief Structure representing a sprite to be rendered.
     * 
     * This structure contains the sprite and its associated transformation matrix for rendering in shadow maps.
     * The sprite is treated similarly to a mesh in terms of transformation, but it may also include special handling 
     * for billboard rendering and animation.
     */
    struct Sprite {
        const R3D_Sprite *sprite;   ///< Pointer to the sprite to be rendered.
        const Matrix transform;     ///< Transformation matrix for the sprite.
    };

    /**
     * @struct ParticlesCPU
     * @brief Structure representing a particle system to be rendered.
     * 
     * This structure contains a pointer to the particle system to be rendered.
     */
    struct ParticlesCPU {
        const R3D_ParticleSystemCPU *system; ///< Pointer to the particle system.
    };

public:
    /**
     * @brief Constructs a draw call for a mesh.
     * 
     * This constructor creates a draw call for rendering a mesh in the shadow map.
     * 
     * @param mesh The mesh to be rendered.
     * @param transform The transformation matrix for the mesh.
     */
    DrawCall_Shadow(const Mesh* mesh, const Matrix& transform);

    /**
     * @brief Constructs a draw call for a sprite.
     * 
     * This constructor creates a draw call for rendering a sprite in the shadow map.
     * The sprite's transformations (position, rotation, scale) will be applied, and its properties will be taken into account 
     * when generating the shadow.
     * 
     * @param sprite The sprite to be rendered.
     * @param transform The transformation matrix for the sprite.
     */
    DrawCall_Shadow(const R3D_Sprite* sprite, const Matrix& transform);

    /**
     * @brief Constructs a draw call for a particle system.
     * 
     * This constructor creates a draw call for rendering a particle system in the shadow map.
     * 
     * @param system The particle system to be rendered.
     */
    DrawCall_Shadow(const R3D_ParticleSystemCPU* system);

    /**
     * @brief Draws the object (either mesh, sprite, or particle system) for shadow mapping.
     * 
     * This method determines which type of object to render (mesh, sprite, or particle system) and calls the appropriate 
     * draw function for shadow mapping.
     * 
     * @param light The light source used for shadow mapping.
     */
    void draw(const Light& light) const;

private:
    /**
     * @brief Draws the mesh for shadow mapping.
     * 
     * This method handles rendering the mesh in the shadow map from the perspective of the specified light source.
     * 
     * @param light The light source used for shadow mapping.
     */
    void drawMesh(const Light& light) const;

    /**
     * @brief Draws the sprite for shadow mapping.
     * 
     * This method handles rendering the sprite in the shadow map. The sprite will be treated similarly to a mesh,
     * with its transformation applied. Special consideration is given to the sprite's properties, such as its billboard mode 
     * and animation, for accurate shadow rendering.
     * 
     * @param light The light source used for shadow mapping.
     */
    void drawSprite(const Light& light) const;

    /**
     * @brief Draws the particle system for shadow mapping.
     * 
     * This method handles rendering the particle system in the shadow map. Each particle in the system will be rendered 
     * in the shadow map with appropriate transformations and lighting interactions.
     * 
     * @param light The light source used for shadow mapping.
     */
    void drawParticlesCPU(const Light& light) const;

private:
    std::variant<Surface, Sprite, ParticlesCPU> mCall; ///< The variant type that holds either a surface (mesh), sprite, or particle system for rendering.
};

/**
 * @brief Stores a draw call for rendering in the main scene.
 * 
 * This class encapsulates a draw call, which can either be a mesh, sprite, or particle system, for rendering in the main scene.
 * It includes the necessary information for applying light influences, material properties, and transformations to render objects in the scene.
 */
class DrawCall_Scene
{
public:
    /**
     * @struct Surface
     * @brief Structure representing a surface to be rendered in the scene.
     * 
     * This structure contains the mesh, material, transformation matrix, and light influences for rendering.
     * It represents a surface (such as a 3D model) that is drawn with a specified material and influenced by the scene's lights.
     */
    struct Surface {
        struct {
            const Mesh *mesh;           ///< Pointer to the mesh to be rendered.
            R3D_Material material;      ///< Material copy for rendering the same mesh with different parameters.
        } surface;                      ///< Surface information for the draw call.
        ShaderLightArray lights;        ///< Array of light pointers influencing this draw call.
        Matrix transform;               ///< Transformation matrix for the draw call.
    };

    /**
     * @struct Sprite
     * @brief Structure representing a sprite to be rendered in the scene.
     * 
     * This structure contains the sprite, material properties, transformation matrix, and light influences for rendering.
     * Sprites are rendered as 2D objects in the 3D scene, and may include special features such as animation or billboard behavior.
     */
    struct Sprite {
        const R3D_Sprite *sprite;       ///< Pointer to the sprite to be rendered.
        ShaderLightArray lights;        ///< Array of light pointers influencing the sprite.
        Matrix transform;               ///< Transformation matrix for the sprite.
    };

    /**
     * @struct ParticlesCPU
     * @brief Structure representing a particle system to be rendered in the scene.
     * 
     * This structure contains the particle system and light influences for rendering.
     * Particle systems allow for rendering dynamic, animated objects that can interact with lights in the scene.
     */
    struct ParticlesCPU {
        R3D_ParticleSystemCPU *system;   ///< Pointer to the particle system to be rendered.
        ShaderLightArray lights;         ///< Array of light pointers influencing the particle system.
    };

public:
    /**
     * @brief Constructs a draw call for a surface to be rendered in the scene.
     * 
     * This constructor creates a draw call for rendering a mesh (surface) in the scene with the provided material, transformation, 
     * and light influences.
     * 
     * @param surface The surface (mesh) to be rendered.
     * @param transform The transformation matrix for the surface.
     * @param lights The array of lights influencing this draw call.
     */
    DrawCall_Scene(const R3D_Surface& surface, const Matrix& transform, const ShaderLightArray& lights);

    /**
     * @brief Constructs a draw call for a sprite to be rendered in the scene.
     * 
     * This constructor creates a draw call for rendering a sprite in the scene. The sprite will be transformed and affected 
     * by the provided lights. Special rendering properties, such as animation or billboard behavior, are handled.
     * 
     * @param sprite The sprite to be rendered.
     * @param transform The transformation matrix for the sprite.
     * @param lights The array of lights influencing the sprite.
     */
    DrawCall_Scene(const R3D_Sprite* sprite, const Matrix& transform, const ShaderLightArray& lights);

    /**
     * @brief Constructs a draw call for a particle system to be rendered in the scene.
     * 
     * This constructor creates a draw call for rendering a particle system in the scene, with the provided light influences 
     * that will affect how the particles are lit.
     * 
     * @param system The particle system to be rendered.
     * @param lights The array of lights influencing the particle system.
     */
    DrawCall_Scene(R3D_ParticleSystemCPU* system, const ShaderLightArray& lights);

    /**
     * @brief Executes the draw call using the provided shader material.
     * 
     * This method performs the actual rendering of the object (mesh, sprite, or particle system) in the scene using the provided 
     * shader material, applying light and material properties to produce the final visual result.
     * 
     * @param shader The shader material to use for rendering.
     */
    void draw(ShaderMaterial& shader) const;

    /**
     * @brief Retrieves the transformation matrix of the surface, if applicable.
     *
     * This method returns the transformation matrix of the object represented by the draw call
     * if it has one, otherwise the function will return `nullptr` if the underlying object does not
     * have a direct transformation, as is the case for particle systems, for example.
     *
     * @return A pointer to the transformation matrix if the draw call object has one, otherwise, `nullptr`.
     */
    const Matrix* getTransform() const;

private:
    /**
     * @brief Draws the mesh for this draw call using the shader material.
     * 
     * This method handles the rendering of the mesh in the scene, applying the material properties, transformation, 
     * and light influences during the drawing process.
     * 
     * @param shader The shader material to use for rendering the mesh.
     */
    void drawMesh(ShaderMaterial& shader) const;

    /**
     * @brief Draws the sprite for this draw call using the shader material.
     * 
     * This method handles the rendering of the sprite in the scene. The sprite's transformation, material properties, 
     * and light influences will be applied during the drawing process. Special rendering features such as animation 
     * or billboard behavior are also taken into account.
     * 
     * @param shader The shader material to use for rendering the sprite.
     */
    void drawSprite(ShaderMaterial& shader) const;

    /**
     * @brief Draws the particle system for this draw call using the shader material.
     * 
     * This method handles the rendering of the particle system in the scene, applying the material properties and light 
     * influences to render the dynamic particles.
     * 
     * @param shader The shader material to use for rendering the particle system.
     */
    void drawParticlesCPU(ShaderMaterial& shader) const;

private:
    std::variant<Surface, Sprite, ParticlesCPU> mCall; ///< The variant type that holds either a surface (mesh), sprite, or particle system for rendering in the scene.
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
    void drawModel(const R3D_Model& model, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector3& scale);

    /**
     * @brief Evaluates and queues a sprite for rendering based on visibility and rendering contexts.
     * 
     * This function performs various tests to determine if the given sprite should be rendered. It applies frustum culling 
     * to check if the sprite is visible within the camera's frustum. Additionally, the sprite is queued for rendering in the 
     * appropriate context, including the main scene or shadow maps. The sprite is rendered using the specified position, rotation, 
     * and scale parameters, and the provided size is used to scale the sprite appropriately.
     * 
     * @param sprite The sprite to evaluate and queue for rendering.
     * @param position The world-space position of the sprite.
     * @param rotationAxis The axis around which the sprite should be rotated.
     * @param rotationAngle The angle of rotation, in degrees, around the specified rotation axis.
     * @param size The size of the sprite in world-space units (scaling the sprite based on this value).
     */
    void drawSprite(const R3D_Sprite& sprite, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector2& size);

    /**
     * @brief Renders the given particle system in the scene.
     * 
     * This method evaluates and renders the specified particle system in the scene. It processes the system's visibility, 
     * handles the required transformations, and queues the appropriate draw calls for rendering. This function is typically 
     * called to render particle systems after their properties (such as position, velocity, and material) have been updated.
     * 
     * @param system The particle system to be rendered.
     */
    void drawParticleSystemCPU(R3D_ParticleSystemCPU& system);

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
    std::array<RenderTarget, 2> mTargetBlur;    ///< Render targets for blurring effects.

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
    , customRenderTarget(nullptr)
    , depthSortingOrder(R3D_DEPTH_SORT_DISABLED)
    , flags(flags)
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

inline void Renderer::drawModel(const R3D_Model& model, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector3& scale)
{
    const std::vector<R3D_Surface>& surfaces = static_cast<Model*>(model.internal)->surfaces;

    // Computes the model's transformation matrix using the provided parameters,
    // the transformation assigned to the model, any parent transformations if present,
    // the transformation specified via rlgl, and also retrieves the model's actual global position.

    Matrix transform = MatrixMultiply(
        MatrixMultiply(
            MatrixScale(scale.x, scale.y, scale.z),
            MatrixRotate(rotationAxis, rotationAngle * DEG2RAD)
        ),
        MatrixTranslate(position.x, position.y, position.z)
    );

    transform = MatrixMultiply(transform, R3D_TransformToGlobal(&model.transform));
    transform = MatrixMultiply(transform, rlGetMatrixTransform());

    const Vector3 modelPosition = getMatrixTrasnlation(transform);

    if (model.billboard != R3D_BILLBOARD_DISABLED) {
        Matrix billboardRotation = getBillboardRotationMatrix(
            model.billboard, modelPosition, mCamera.position
        );
        transform = MatrixMultiply(transform, billboardRotation);
    }

    // Retrieves the model's bounding box transformed according to the previously obtained transformation matrix.

    BoundingBox globalAABB = model.aabb;
    globalAABB.min = Vector3Transform(globalAABB.min, transform);
    globalAABB.max = Vector3Transform(globalAABB.max, transform);

    // Performs a bounding box test for the model against the camera's frustum if necessary
    // to determine if the object should be rendered in the visible scene. This feature can be
    // disabled using the initialization flag 'R3D_FLAG_NO_FRUSTUM_CULLING'.

    bool drawSurfaceScene = true;
    if (!(flags & R3D_FLAG_NO_FRUSTUM_CULLING || model.shadow == R3D_CAST_SHADOW_ONLY)) {
        drawSurfaceScene = mFrustumCamera.aabbIn(globalAABB);
    }

    // Determines which lights will interact with the model.
    // This part checks which lights should cast shadows onto the model
    // and also associates the lights with the draw call if the model is to be rendered in the scene.

    ShaderLightArray lightsWhichShouldIlluminate{};
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
                            batch.push_back(DrawCall_Shadow(&surface.mesh, transform));
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
                            batch.push_back(DrawCall_Shadow(&surface.mesh, transform));
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
                            batch.push_back(DrawCall_Shadow(&surface.mesh, transform));
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
            mSceneBatches.pushDrawCall(
                surface.material.config,
                DrawCall_Scene(surface, transform, lightsWhichShouldIlluminate)
            );
        }
    }
}

inline void Renderer::drawSprite(const R3D_Sprite& sprite, const Vector3& position, const Vector3& rotationAxis, float rotationAngle, const Vector2& size)
{
    // Computes the sprite's transformation matrix using the provided parameters,
    // the transformation assigned to the sprite, any parent transformations if present,
    // the transformation specified via rlgl, and also retrieves the sprite's actual global position.

    Matrix transform = MatrixMultiply(
        MatrixMultiply(
            MatrixScale(size.x * 0.5f, size.y * 0.5f, 1.0f),
            MatrixRotate(rotationAxis, rotationAngle * DEG2RAD)
        ),
        MatrixTranslate(position.x, position.y, position.z)
    );

    transform = MatrixMultiply(transform, R3D_TransformToGlobal(&sprite.transform));
    transform = MatrixMultiply(transform, rlGetMatrixTransform());

    const Vector3 centerPosition = getMatrixTrasnlation(transform);

    if (sprite.billboard != R3D_BILLBOARD_DISABLED) {
        Matrix billboardRotation = getBillboardRotationMatrix(
            sprite.billboard, centerPosition, mCamera.position
        );
        transform = MatrixMultiply(transform, billboardRotation);
    }

    // Retrieves the sprite's bounding box transformed according to the previously obtained transformation matrix.

    BoundingBox globalAABB = {
        { -1.0f, -1.0f, -1.0f },
        { 1.0f, 1.0f, 1.0f }
    };
    globalAABB.min = Vector3Transform(globalAABB.min, transform);
    globalAABB.max = Vector3Transform(globalAABB.max, transform);

    // Performs a bounding box test for the sprite against the camera's frustum if necessary
    // to determine if the object should be rendered in the visible scene. This feature can be
    // disabled using the initialization flag 'R3D_FLAG_NO_FRUSTUM_CULLING'.

    bool drawSurfaceScene = true;
    if (!(flags & R3D_FLAG_NO_FRUSTUM_CULLING || sprite.shadow == R3D_CAST_SHADOW_ONLY)) {
        drawSurfaceScene = mFrustumCamera.aabbIn(globalAABB);
    }

    // Determines which lights will interact with the sprite.
    // This part checks which lights should cast shadows onto the sprite
    // and also associates the lights with the draw call if the sprite is to be rendered in the scene.

    ShaderLightArray lightsWhichShouldIlluminate{};
    int lightsWhichShouldIlluminateCount = 0;

    for (const auto& [id, light] : mLights) {

        if (!light.enabled) continue;

        // Here, we use a rather naive approach. If the light is not directional—
        // for instance, it does not simulate the sun—then we check whether the squared 
        // distance between the origin of the sprite and the light exceeds the maximum range 
        // of the light's effective field. If it does, we can safely omit this light source for the sprite.

        // Using the squared distance is faster to compute and provides a certain margin, 
        // making it acceptable in this context.

        const float lightMaxDistSqr = light.maxDistance * light.maxDistance;
        if (light.type != R3D_DIRLIGHT && Vector3DistanceSqr(centerPosition, light.position) > lightMaxDistSqr) {
            continue;
        }

        // Here we add:
        //   - 1: The surfaces to the shadow batch for the light
        //   - 2: The light to the surface draw calls for the scene rendering

        auto& batch = mShadowBatches.getBatch(id);

        switch (light.type) {
            case R3D_DIRLIGHT: {
                if (light.frustum.aabbIn(globalAABB)) {
                    if (sprite.shadow != R3D_CAST_OFF && light.shadow) {
                        batch.push_back(DrawCall_Shadow(&sprite, transform));
                    }
                    if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                        lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                        lightsWhichShouldIlluminateCount++;
                    }
                }
            } break;
            case R3D_SPOTLIGHT: {
                if (light.frustum.aabbIn(globalAABB)) {
                    if (sprite.shadow != R3D_CAST_OFF && light.shadow) {
                        batch.push_back(DrawCall_Shadow(&sprite, transform));
                    }
                    if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                        lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                        lightsWhichShouldIlluminateCount++;
                    }
                }
            } break;
            case R3D_OMNILIGHT: {
                // If we have reached this point, it means that the sprite is within the maximum influence distance of the omni light. 
                // Therefore, we don't need to perform a frustum test since it illuminates in all directions.
                if (sprite.shadow != R3D_CAST_OFF && light.shadow) {
                    batch.push_back(DrawCall_Shadow(&sprite, transform));
                }
                if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                    lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                    lightsWhichShouldIlluminateCount++;
                }
            } break;
        }
    }

    if (drawSurfaceScene) {
        mSceneBatches.pushDrawCall(
            sprite.material.config,
            DrawCall_Scene(&sprite, transform, lightsWhichShouldIlluminate)
        );
    }
}

inline void Renderer::drawParticleSystemCPU(R3D_ParticleSystemCPU& system)
{
    // Computes the system's transformation matrix using the provided parameters,
    // the transformation assigned to the system, any parent transformations if present,
    // the transformation specified via rlgl, and also retrieves the system's actual global position.

    Matrix transform = MatrixTranslate(system.position.x, system.position.y, system.position.z);
    transform = MatrixMultiply(transform, rlGetMatrixTransform());

    // Retrieves the system's bounding box transformed according to the previously obtained transformation matrix.

    BoundingBox globalAABB = system.aabb;
    globalAABB.min = Vector3Transform(globalAABB.min, transform);
    globalAABB.max = Vector3Transform(globalAABB.max, transform);

    // Performs a bounding box test for the system against the camera's frustum if necessary
    // to determine if the object should be rendered in the visible scene. This feature can be
    // disabled using the initialization flag 'R3D_FLAG_NO_FRUSTUM_CULLING'.

    bool drawSurfaceScene = true;
    if (!(flags & R3D_FLAG_NO_FRUSTUM_CULLING || system.shadow == R3D_CAST_SHADOW_ONLY)) {
        drawSurfaceScene = mFrustumCamera.aabbIn(globalAABB);
    }

    // Determines which lights will interact with the system.
    // This part checks which lights should cast shadows onto the system
    // and also associates the lights with the draw call if the system is to be rendered in the scene.

    ShaderLightArray lightsWhichShouldIlluminate{};
    int lightsWhichShouldIlluminateCount = 0;

    for (const auto& [id, light] : mLights) {

        if (!light.enabled) continue;

        // Here, we use a rather naive approach. If the light is not directional—
        // for instance, it does not simulate the sun—then we check whether the squared 
        // distance between the origin of the system and the light exceeds the maximum range 
        // of the light's effective field. If it does, we can safely omit this light source for the system.

        // Using the squared distance is faster to compute and provides a certain margin, 
        // making it acceptable in this context.

        const float lightMaxDistSqr = light.maxDistance * light.maxDistance;
        if (light.type != R3D_DIRLIGHT && Vector3DistanceSqr(system.position, light.position) > lightMaxDistSqr) {
            continue;
        }

        // Here we add:
        //   - 1: The surfaces to the shadow batch for the light
        //   - 2: The light to the surface draw calls for the scene rendering

        auto& batch = mShadowBatches.getBatch(id);

        switch (light.type) {
            case R3D_DIRLIGHT: {
                if (light.frustum.aabbIn(globalAABB)) {
                    if (system.shadow != R3D_CAST_OFF && light.shadow) {
                        batch.push_back(DrawCall_Shadow(&system));
                    }
                    if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                        lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                        lightsWhichShouldIlluminateCount++;
                    }
                }
            } break;
            case R3D_SPOTLIGHT: {
                if (light.frustum.aabbIn(globalAABB)) {
                    if (system.shadow != R3D_CAST_OFF && light.shadow) {
                        batch.push_back(DrawCall_Shadow(&system));
                    }
                    if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                        lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                        lightsWhichShouldIlluminateCount++;
                    }
                }
            } break;
            case R3D_OMNILIGHT: {
                // If we have reached this point, it means that the system is within the maximum influence distance of the omni light. 
                // Therefore, we don't need to perform a frustum test since it illuminates in all directions.
                if (system.shadow != R3D_CAST_OFF && light.shadow) {
                        batch.push_back(DrawCall_Shadow(&system));
                }
                if (drawSurfaceScene && lightsWhichShouldIlluminateCount < SHADER_LIGHT_COUNT) {
                    lightsWhichShouldIlluminate[lightsWhichShouldIlluminateCount] = &light;
                    lightsWhichShouldIlluminateCount++;
                }
            } break;
        }
    }

    if (drawSurfaceScene) {
        mSceneBatches.pushDrawCall(
            system.surface.material.config,
            DrawCall_Scene(&system, lightsWhichShouldIlluminate)
        );
    }
}

inline void Renderer::present()
{
    rlDrawRenderBatchActive();
    rlEnableDepthTest();

    /* Shadow casting */

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

    /* Sorting scene depth if necessary */
    
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


    /* Render scene */

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

        /* Render surfaces */

        for (auto& [config, batch] : mSceneBatches) {
            if (batch.empty()) continue;

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

    bool blitLinear = R3D_FLAG_BLIT_LINEAR;
    GLint target = customRenderTarget ? customRenderTarget->id : 0;

    if (flags & R3D_FLAG_ASPECT_KEEP) {
        mTargetPostFX.blitAspectKeep(target, GLAttachement::COLOR_0, false, blitLinear);
        mTargetScene.blitAspectKeep(target, GLAttachement::NONE, true, false);
    } else {
        mTargetPostFX.blitAspectExpand(target, GLAttachement::COLOR_0, false, blitLinear);
        mTargetScene.blitAspectExpand(target, GLAttachement::NONE, true, false);
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

inline void Renderer::drawMeshShadow(const Light& light, const Mesh& mesh, const Matrix& transform) const
{
    Matrix matView = rlGetMatrixModelview();

    Matrix matModelView = MatrixMultiply(transform, matView);
    Matrix matProjection = rlGetMatrixProjection();

    if (light.type == R3D_OMNILIGHT) {
        rlSetUniform(mShaderDepthCube.locs[SHADER_LOC_VECTOR_VIEW], &light.position, SHADER_UNIFORM_VEC3, 1);
        rlSetUniformMatrix(mShaderDepthCube.locs[SHADER_LOC_MATRIX_MODEL], transform);
    }

    if (!rlEnableVertexArray(mesh.vaoId)) {
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
        if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] > 0) {
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

        if (mesh.vboId[RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES] == 0) {
            rlDrawVertexArray(0, mesh.vertexCount);
        } else {
            rlDrawVertexArrayElements(0, 3 * mesh.triangleCount, 0);
        }
    }

    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
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

inline DrawCall_Scene::DrawCall_Scene(R3D_ParticleSystemCPU* system, const ShaderLightArray& lights)
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

        call.system->surface.material.albedo.color = (Color) {
            static_cast<uint8_t>((baseColor.r * particle.color.r) / 255),
            static_cast<uint8_t>((baseColor.g * particle.color.g) / 255),
            static_cast<uint8_t>((baseColor.b * particle.color.b) / 255),
            static_cast<uint8_t>((baseColor.a * particle.color.a) / 255)
        };

        shader.setMaterial(call.system->surface.material);
        shader.setMatModel(transform);
        shader.setLights(call.lights);

        gRenderer->drawMeshScene(call.system->surface.mesh, transform, shader, call.system->surface.material.config);
    }

    call.system->surface.material.albedo.color = baseColor;
}

} // namespace r3d

#endif // R3D_RENDERER_HPP
