#ifndef R3D_H
#define R3D_H

#include <raylib.h>


/* Enums */

/**
 * @enum R3D_Flags
 * @brief Flags for configuring R3D rendering behavior.
 * 
 * These flags control various aspects of the internal rendering pipeline, 
 * such as scaling, aspect ratio management, and debugging tools.
 */
typedef enum {
    R3D_FLAG_NONE               = 0,        /**< No flags are set. */

    R3D_FLAG_BLIT_LINEAR        = 1 << 0,   /**< Indicates whether internal framebuffers should be scaled
                                             *   linearly during the blit operation to the main framebuffer.
                                             *   The default mode used is 'GL_NEAREST'.
                                             */

    R3D_FLAG_ASPECT_KEEP        = 1 << 1,   /**< Specifies whether the aspect ratio of the internal resolution 
                                             *   should be preserved when scaling to the window size. This mode 
                                             *   simulates letterboxing or pillarboxing by adding black bars 
                                             *   to fill empty space. In the default mode, the internal buffer 
                                             *   is stretched to fill the window surface, adapting to the 
                                             *   window's aspect ratio rather than the internal resolution's.
                                             */

    R3D_FLAG_NO_FRUSTUM_CULLING = 1 << 2,   /**< Indicates that frustum culling should not be performed on objects
                                             *   rendered in the final scene. Useful if you are already using a 
                                             *   more project-specific algorithm. However, this parameter is not 
                                             *   applied to lights producing shadows; frustum culling will always 
                                             *   be performed to determine if an object should be rendered in a 
                                             *   shadow map.
                                             */

    R3D_FLAG_DEBUG_SHADOW_MAP   = 1 << 3,   /**< Indicates whether the shadow map rendering shaders should be 
                                             *   loaded. If this flag is not set, calling `R3D_DrawShadowMap` 
                                             *   will have no effect.
                                             */
} R3D_Flags;

/**
 * @enum R3D_Bloom
 * @brief Defines the types of bloom effects available for rendering.
 * 
 * Bloom is a post-processing effect used to simulate the appearance of light bleeding
 * beyond its original boundaries, creating a glowing halo around bright areas.
 */
typedef enum {
    R3D_BLOOM_DISABLED,         /**< Bloom effect is disabled. */
    R3D_BLOOM_ADDITIVE,         /**< Additive bloom effect, where bright areas are enhanced by adding light to them. */
    R3D_BLOOM_SOFT_LIGHT        /**< Soft light bloom effect, which creates a softer, more diffused glow around bright areas. */
} R3D_Bloom;

/**
 * @enum R3D_Fog
 * @brief Defines the types of fog effects available for rendering.
 * 
 * Fog is a visual effect used to simulate the presence of atmospheric particles,
 * reducing visibility and adding depth to a scene.
 */
typedef enum {
    R3D_FOG_DISABLED,           /**< Fog effect is disabled. */
    R3D_FOG_LINEAR,             /**< Linear fog, where the density increases linearly based on distance from the camera. */
    R3D_FOG_EXP2,               /**< Exponential fog (exp2), where the density increases exponentially with distance. */
    R3D_FOG_EXP,                /**< Exponential fog, where the density increases exponentially but at a different rate compared to EXP2. */
} R3D_Fog;

/**
 * @enum R3D_Tonemap
 * @brief Defines the available tone-mapping algorithms for rendering.
 * 
 * Tone mapping is a technique used to convert high dynamic range (HDR) lighting to a
 * low dynamic range (LDR) display, allowing for more realistic lighting effects.
 */
typedef enum {
    R3D_TONEMAP_LINEAR,         /**< Linear tone mapping, which performs a simple linear mapping of HDR values. */
    R3D_TONEMAP_REINHARD,       /**< Reinhard tone mapping, a popular algorithm for compressing HDR values. */
    R3D_TONEMAP_FILMIC,         /**< Filmic tone mapping, which simulates the response of film to light. */
    R3D_TONEMAP_ACES,           /**< ACES (Academy Color Encoding System) tone mapping, a high-quality algorithm used for cinematic rendering. */
} R3D_Tonemap;

/**
 * @enum R3D_DiffuseMode
 * @brief Defines the different methods for calculating diffuse lighting in the rendering engine.
 * 
 * Diffuse lighting determines how light interacts with a surface, scattering in all directions. 
 * These modes allow for different shading models to be applied for rendering objects.
 */
typedef enum {
    R3D_DIFFUSE_UNSHADED,           /**< Unshaded, only albedo, vertex colors, blend mode, and cull mode will be considered. */
    R3D_DIFFUSE_BURLEY,             /**< Burley diffuse model, an approximation of the Lambertian reflectance model for diffuse lighting. */
    R3D_DIFFUSE_DISNEY,             /**< Disney diffuse model, part of the Disney BRDF, providing a more realistic shading model. */
    R3D_DIFFUSE_LAMBERT,            /**< Lambertian diffuse model, based on the cosine of the angle between the light source and the surface normal. */
    R3D_DIFFUSE_PHONG,              /**< Phong diffuse model, a more advanced model with a focus on simulating smoother shading. */
    R3D_DIFFUSE_TOON,               /**< Toon shading for diffuse light, providing a stylized, flat shading effect with sharp color transitions. */
} R3D_DiffuseMode;

/**
 * @enum R3D_SpecularMode
 * @brief Defines the different methods for calculating specular reflection in the rendering engine.
 * 
 * Specular reflection models how light reflects off shiny surfaces, creating highlights. 
 * These modes provide various ways of simulating this effect.
 */
typedef enum {
    R3D_SPECULAR_DISABLED,          /**< Specular reflection is disabled. No specular highlights will be calculated. */
    R3D_SPECULAR_SCHLICK_GGX,       /**< Schlick-GGX specular model, a physically-based model that approximates specular reflection. */
    R3D_SPECULAR_DISNEY,            /**< Disney specular model, part of the Disney BRDF, for realistic specular highlights with energy conservation. */
    R3D_SPECULAR_BLINN_PHONG,       /**< Blinn-Phong specular model, an approximation of the Phong model that improves performance. */
    R3D_SPECULAR_TOON,              /**< Toon shading for specular reflection, creating stylized highlights for a cartoon-like appearance. */
} R3D_SpecularMode;

/**
 * @enum R3D_BlendMode
 * @brief Defines the different blending modes for textures in the rendering engine.
 * 
 * Blending determines how textures are combined with the background, influencing transparency, 
 * color addition, multiplication, and other visual effects.
 */
typedef enum {
    R3D_BLEND_DISABLED,                 /**< No blending, textures are rendered without any transparency or color mixing. */
    R3D_BLEND_ALPHA,                    /**< Blend textures considering the alpha channel (default mode). Used for transparency effects. */
    R3D_BLEND_ADDITIVE,                 /**< Additive blending, where the colors of the textures are added together, useful for glowing effects. */
    R3D_BLEND_MULTIPLIED,               /**< Multiplied blending, where the colors of the textures are multiplied together, producing darker results. */
    R3D_BLEND_ADD_COLORS,               /**< Additive blending with an alternative approach, adding the colors of the textures together. */
    R3D_BLEND_SUBTRACT_COLORS,          /**< Subtractive blending, where the colors of the textures are subtracted, creating inverted effects. */
    R3D_BLEND_ALPHA_PREMULTIPLY,        /**< Premultiplied alpha blending, used when textures have already been pre-multiplied by their alpha value. */
} R3D_BlendMode;

/**
 * @enum R3D_CullMode
 * @brief Defines the different culling modes for geometry in the rendering engine.
 * 
 * Culling is used to determine which faces of a 3D object should be rendered based on their orientation.
 * This is used to improve performance by discarding geometry that is not visible to the camera.
 */
typedef enum {
    R3D_CULL_DISABLED,                  /**< No culling, all faces are rendered regardless of orientation. */
    R3D_CULL_FRONT,                     /**< Cull front faces, rendering only the back faces of the geometry. */
    R3D_CULL_BACK,                      /**< Cull back faces, rendering only the front faces of the geometry. */
} R3D_CullMode;

/**
 * @enum R3D_MaterialFlags
 * @brief Defines flags used to configure material properties in the rendering engine.
 * 
 * These flags control various material properties such as whether the material uses
 * vertex colors, receives shadows, or has specific texture maps (e.g., emission, normal, ambient occlusion).
 * 
 * @note The maximum value of this enum must not exceed the range of a `uint8_t`. Internally, 
 *       `R3D_MaterialConfig` is stored as a `uint64_t`, and `R3D_MaterialShaderConfig` is stored as a 
 *       `uint32_t`, both of which are used as keys for sorting materials and shaders.
 */
typedef enum {
    R3D_MATERIAL_FLAG_NONE              = 0,          /**< No material flags set. */
    R3D_MATERIAL_FLAG_VERTEX_COLOR      = 1 << 0,     /**< Material uses vertex colors for shading. */
    R3D_MATERIAL_FLAG_RECEIVE_SHADOW    = 1 << 1,     /**< Material will receive shadows in the rendering process. */
    R3D_MATERIAL_FLAG_MAP_EMISSION      = 1 << 2,     /**< Material uses an emission map (emissive texture). */
    R3D_MATERIAL_FLAG_MAP_NORMAL        = 1 << 3,     /**< Material uses a normal map to modify surface normals. */
    R3D_MATERIAL_FLAG_MAP_AO            = 1 << 4,     /**< Material uses an ambient occlusion map to simulate light occlusion. */
    R3D_MATERIAL_FLAG_SKY_IBL           = 1 << 5,     /**< Material uses sky-based image-based lighting (IBL). */
} R3D_MaterialFlags;

/**
 * @enum R3D_CastShadow
 * @brief Defines how shadows are handled for a specific `R3D_Model`.
 * 
 * This enum specifies whether the model will cast shadows, receive shadows, or be treated in a specific way
 * with regard to shadows in the rendering process.
 */
typedef enum {
    R3D_CAST_OFF,            /**< The renderer will not cast shadows for this mesh. */
    R3D_CAST_ON,             /**< The renderer will cast shadows for this mesh. */
    R3D_CAST_SHADOW_ONLY     /**< The mesh will not be rendered, but it will cast shadows. */
} R3D_CastShadow;

/**
 * @enum R3D_LightType
 * @brief Defines the types of lights available in the rendering engine.
 * 
 * Each light type has a specific role in creating realistic lighting effects in the scene.
 */
typedef enum {
    R3D_DIRLIGHT = 0,        /**< Directional light, simulating a distant light source such as the sun. */
    R3D_SPOTLIGHT,           /**< Spotlight, emitting light in a cone-shaped area. */
    R3D_OMNILIGHT            /**< Omnidirectional light, emitting light in all directions from a single point. */
} R3D_LightType;


/* Structs */

/**
 * @typedef R3D_Skybox
 * @brief Represents an opaque type for handling skyboxes in the rendering engine.
 * 
 * A skybox is a cube-mapped texture used to create the illusion of a distant environment, such as a sky or space.
 * The actual implementation is opaque and managed internally by the rendering engine.
 */
typedef void* R3D_Skybox;

/**
 * @struct R3D_Environment
 * @brief Encapsulates environment settings for rendering, including bloom effects, fog, tone mapping, 
 *        color adjustments, and skybox configuration.
 * 
 * This structure provides a comprehensive way to configure the rendering environment,
 * enabling features like bloom, fog, tone mapping, and world appearance settings.
 */
typedef struct {
    struct {
        R3D_Bloom mode;         /**< The bloom effect mode. Default: `R3D_BLOOM_DISABLED`. */
        float intensity;        /**< Intensity of the bloom effect. Default: `1.0f`. */
        float hdrThreshold;     /**< HDR brightness threshold for the bloom effect. Default: `1.0f`. */
        int iterations;         /**< Number of iterations for the bloom effect. Default: `10`. */
    } bloom;                    /**< Configuration for bloom effects. */

    struct {
        R3D_Fog mode;           /**< Fog rendering mode. Default: `R3D_FOG_DISABLED`. */
        Color color;            /**< Fog color. Default: `GRAY`. */
        float start;            /**< Fog start distance (used only in `linear` mode). Default: `10.0f`. */
        float end;              /**< Fog end distance (used only in `linear` mode). Default: `30.0f`. */
        float density;          /**< Fog density (used in `exp` and `exp2` modes). Default: `0.1f`. */
    } fog;                      /**< Configuration for fog rendering. */

    struct {
        R3D_Tonemap mode;       /**< Tone mapping mode. Default: `R3D_TONEMAP_LINEAR`. */
        float exposure;         /**< Exposure for tone mapping. Default: `1.0f`. */
        float white;            /**< White point adjustment (not used in `linear` mode). Default: `1.0f`. */
    } tonemap;                  /**< Configuration for tone mapping. */

    struct {
        float brightness;       /**< Brightness adjustment. Default: `1.0f`. */
        float contrast;         /**< Contrast adjustment. Default: `1.0f`. */
        float saturation;       /**< Saturation adjustment. Default: `1.0f`. */
    } adjustements;             /**< Configuration for post-processing color adjustments. */

    struct {
        R3D_Skybox skybox;      /**< Skybox used for rendering the world environment. Default: `NULL`. */
        Color background;       /**< Background color (used only when no skybox is defined). Default: `GRAY`. */
        Color ambient;          /**< Ambient color (used only when no skybox is defined). Default: `DARKGRAY`. */
    } world;                    /**< Configuration for the world environment, including skybox and ambient settings. */
} R3D_Environment;

/**
 * @struct R3D_MaterialShaderConfig
 * @brief Configuration for material shaders, including diffuse, specular, and additional flags.
 * 
 * This structure is used internally to determine the shader configuration for a material.
 * It allows fine-grained control over shading parameters.
 */
typedef struct {
    unsigned char diffuse;     /**< The diffuse mode for the material (see `R3D_DiffuseMode`). */
    unsigned char specular;    /**< The specular mode for the material (see `R3D_SpecularMode`). */
    unsigned char reserved;    /**< Reserved for future use. */
    unsigned char flags;       /**< Flags indicating additional shader settings. */
} R3D_MaterialShaderConfig;

/**
 * @struct R3D_MaterialConfig
 * @brief Encapsulates the overall configuration for a material, including blending, culling, and shader settings.
 * 
 * This structure holds essential rendering configurations for materials and is tied to both
 * blending and culling modes, along with shader-specific options.
 */
typedef struct {
    R3D_MaterialShaderConfig shader;  /**< Shader configuration for the material. */
    unsigned char blendMode;          /**< Blending mode for the material (see `R3D_BlendMode`). */
    unsigned char cullMode;           /**< Culling mode for the material (see `R3D_CullMode`). */
    unsigned char reserved1;          /**< Reserved for future use. */
    unsigned char reserved2;          /**< Reserved for future use. */
} R3D_MaterialConfig;

/**
 * @struct R3D_Material
 * @brief Defines a material with textures, colors, and rendering configurations.
 * 
 * This structure is used to represent a material in the rendering engine, including its texture maps, 
 * color properties, and material-specific configurations such as blending, culling, and shading parameters.
 * 
 * @note To modify the `config` field, use `R3D_UpdateMaterialConfig` to ensure consistency.
 */
typedef struct {
    struct {
        Texture2D texture;      /**< The albedo texture. Default: White texture. */
        Color color;            /**< The albedo color. Default: White color. */
    } albedo;                   /**< Albedo properties of the material. */

    struct {
        Texture2D texture;      /**< The metalness texture. Default: White texture. */
        float factor;           /**< The metalness factor. Default: `0.0f`. */
    } metalness;                /**< Metalness properties of the material. */

    struct {
        Texture2D texture;      /**< The roughness texture. Default: White texture. */
        float factor;           /**< The roughness factor. Default: `1.0f`. */
    } roughness;                /**< Roughness properties of the material. */

    struct {
        Texture2D texture;      /**< The emission texture. Default: Black texture. */
        float energy;           /**< The emission intensity (energy). Default: `1.0f`. */
        Color color;            /**< The emission color. Default: Black color. */
    } emission;                 /**< Emission properties of the material. */

    struct {
        Texture2D texture;      /**< The normal map texture. Default: Black texture. */
    } normal;                   /**< Normal mapping properties of the material. */

    struct {
        Texture2D texture;      /**< The ambient occlusion (AO) texture. Default: White texture. */
        float lightAffect;      /**< Factor determining the light's impact on AO. Default: `0.0f`. */
    } ao;                       /**< Ambient occlusion properties of the material. */

    R3D_MaterialConfig config;  /**< Material configuration. Do not modify directly; use `R3D_UpdateMaterialConfig` instead. */
} R3D_Material;

/**
 * @struct R3D_Transform
 * @brief Represents a 3D transformation including position, rotation, scale, and optional hierarchical parenting.
 * 
 * This structure is used to define the spatial properties of an object in the 3D world.
 * A transform can optionally inherit transformations from a parent, enabling hierarchical relationships.
 */
typedef struct R3D_Transform {
    Vector3 position;                   /**< Position of the object in world space. */
    Quaternion rotation;                /**< Rotation of the object, represented as a quaternion. */
    Vector3 scale;                      /**< Scale of the object along each axis. */
    const struct R3D_Transform *parent; /**< Pointer to the parent transform for hierarchical relationships. */
} R3D_Transform;

/**
 * @struct R3D_Surface
 * @brief Represents a renderable surface, combining a material and a mesh.
 * 
 * A surface consists of a mesh, which defines its geometry, and a material, which determines its appearance.
 */
typedef struct {
    R3D_Material material;           /**< Material defining the surface's appearance. */
    Mesh mesh;                       /**< Mesh defining the surface's geometry. */
} R3D_Surface;

/**
 * @struct R3D_Model
 * @brief Represents a 3D model in the rendering engine, including its transform, bounding box, shadow settings, and internal data.
 * 
 * A model is a higher-level abstraction that encapsulates the geometry, spatial properties, and shadow casting behavior.
 * It can also include additional internal data for rendering optimizations.
 */
typedef struct {
    R3D_Transform transform;         /**< Transform defining the position, rotation, and scale of the model. */
    BoundingBox aabb;                /**< Axis-aligned bounding box (AABB) for the model. Used for culling and spatial queries. */
    R3D_CastShadow shadow;           /**< Shadow casting behavior of the model (see `R3D_CastShadow`). */
    void *internal;                  /**< Internal data used by the rendering engine. Should not be modified directly. */
} R3D_Model;

typedef unsigned int R3D_Light;

#ifdef __cplusplus
extern "C" {
#endif

// DOC: Tous les objets provenant du module 'core' n'ont pas besoin de destruction explicite et seront detruit à l'appel de 'R3D_Close();'


/* [Core] Main functions */

/**
 * @brief Initializes the R3D rendering engine with default settings.
 * 
 * This function sets up the R3D engine using the default window resolution for internal rendering
 * and no additional flags. It must be called before using any other R3D functions.
 * 
 * @note All objects created by the Core module do not require explicit destruction. 
 *       They are automatically cleaned up when `R3D_Close` is called.
 */
void R3D_Init(void);

/**
 * @brief Initializes the R3D rendering engine with custom settings.
 * 
 * This function allows the specification of the internal rendering resolution and flags. 
 * It should be used if the default configuration provided by `R3D_Init` does not meet the application's needs.
 * 
 * @param internalWidth  The internal rendering width. If `<= 0`, the primary framebuffer's width will be used.
 * @param internalHeight The internal rendering height. If `<= 0`, the primary framebuffer's height will be used.
 * @param flags          Additional configuration flags for the renderer (see `R3D_Flags`).
 *
 * @note All objects created by the Core module do not require explicit destruction. 
 *       They are automatically cleaned up when `R3D_Close` is called.
 */
void R3D_InitEx(int internalWidth, int internalHeight, int flags);

/**
 * @brief Closes the R3D rendering engine and releases all associated resources.
 * 
 * This function cleans up all objects and resources managed by the R3D engine. 
 * It must be called before exiting the application to ensure proper cleanup.
 * 
 * @note All objects created by the Core module are automatically destroyed when this function is called.
 */
void R3D_Close(void);

/**
 * @brief Updates the resolution of the internal render targets.
 * 
 * This function changes the resolution used for the internal rendering process. If the provided 
 * width or height is set to `<= 0`, the resolution of the primary framebuffer will be used instead.
 * 
 * @param width  The desired internal rendering width. Use `<= 0` to match the primary framebuffer width.
 * @param height The desired internal rendering height. Use `<= 0` to match the primary framebuffer height.
 * 
 * @note Updating the internal resolution will impact rendering performance and quality.
 */
void R3D_UpdateInternalResolution(int width, int height);

/**
 * @brief Configures the blitting mode used when rendering the internal buffer to the main framebuffer.
 * 
 * This function controls how the internal render buffer is displayed on the primary framebuffer, 
 * allowing customization of aspect ratio preservation and scaling method.
 * 
 * @param blitAspectKeep If true, the aspect ratio of the internal resolution is preserved during blitting. 
 *                       Black bars may appear on the sides to fill the empty space. If false, the internal 
 *                       buffer is stretched to fit the window, using the window's aspect ratio.
 * @param blitLinear     If true, the `GL_LINEAR` mode is used for smooth scaling. If false, the `GL_NEAREST` 
 *                       mode is used for pixel-perfect scaling.
 * 
 * @note The default mode uses `GL_NEAREST` for scaling without aspect ratio preservation.
 */
void R3D_SetBlitMode(bool blitAspectKeep, bool blitLinear);

/**
 * @brief Enables or disables frustum culling for objects rendered in the final visible scene.
 * 
 * This function allows you to enable or disable frustum culling, a technique used to exclude objects
 * that are outside of the camera's viewing frustum from the rendering process. This can improve performance
 * by reducing the number of objects that need to be drawn. It is particularly useful if you are already
 * performing your own culling tests before calling `R3D_Draw()`.
 * 
 * @param enabled If `true`, frustum culling will be enabled. If `false`, frustum culling will be disabled.
 * 
 * @note This option does not apply to directional lights and spotlights that produce shadows. 
 *       Frustum culling will still be performed from their respective viewpoints to determine if
 *       an object should be rendered in their shadow maps.
 */
void R3D_SetFrustumCulling(bool enabled);

/**
 * @brief Sets the render target for the R3D rendering process.
 * 
 * This function allows you to specify a custom render target where the rendering done by R3D 
 * will be blitted, instead of the default main framebuffer. This is useful when you want to 
 * render to off-screen targets for post-processing or other effects.
 * 
 * @param target A pointer to the render target (RenderTexture) where the rendering will be directed.
 *               Pass `NULL` to restore the main framebuffer as the render target.
 * 
 * @note If `target` is `NULL`, the rendering will be directed back to the main framebuffer.
 */
void R3D_SetRenderTarget(const RenderTexture* target);

/**
 * @brief Begins a new rendering frame using the R3D engine with the specified camera.
 * 
 * This function sets up the rendering process for the current frame. It prepares the internal rendering
 * targets and ensures all subsequent drawing operations are properly accumulated for post-processing.
 * 
 * @param camera The 3D camera to use for rendering the scene.
 * 
 * @note You cannot use `BeginTextureMode` to capture rendering performed by R3D into another framebuffer.
 *       However, you can use `R3D_SetRenderTarget` to specify an alternative render target (other than the main framebuffer) 
 *       for R3D's rendering operations. This allows you to perform off-screen rendering before finalizing the frame.
 *       This restriction exists because R3D manages its own internal render targets for post-processing 
 *       and shadow mapping. All drawing operations should be performed using `R3D_Draw` and finalized 
 *       with `R3D_End`.
 * 
 * @note This approach simplifies the API but is currently incompatible with the `rshapes.c` module of raylib.
 */
void R3D_Begin(Camera3D camera);

/**
 * @brief Draws a 3D model using its defined transformation.
 * 
 * This function renders the specified model using the transformation defined in its `R3D_Transform`. 
 * No additional transformations are applied.
 * 
 * @param model A pointer to the `R3D_Model` to be drawn.
 * 
 * @note Ensure the model's `R3D_Transform` is correctly configured before calling this function.
 */
void R3D_Draw(const R3D_Model* model);

/**
 * @brief Draws a 3D model with a specified position and uniform scaling.
 * 
 * This function renders the specified model at the given position with a uniform scale factor.
 * The model's internal transformation is ignored.
 * 
 * @param model   A pointer to the `R3D_Model` to be drawn.
 * @param position The position where the model should be rendered.
 * @param scale    A uniform scale factor to apply to the model.
 */
void R3D_DrawEx(const R3D_Model* model, Vector3 position, float scale);

/**
 * @brief Draws a 3D model with advanced transformation settings.
 * 
 * This function allows rendering a model at a specified position, with custom scaling and rotation.
 * The model's internal transformation is ignored.
 * 
 * @param model        A pointer to the `R3D_Model` to be drawn.
 * @param position     The position where the model should be rendered.
 * @param rotationAxis The axis around which the model should be rotated.
 * @param rotationAngle The angle of rotation (in degrees) around the specified axis.
 * @param scale        A scaling factor to apply to the model in each axis.
 */
void R3D_DrawPro(const R3D_Model* model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale);

/**
 * @brief Finalizes the current rendering frame.
 * 
 * This function completes the rendering process for the current frame.
 * It iterates over all surfaces, rendering them first to the shadow maps, then to the scene render target,
 * and finally applies all post-processing effects and blits the result to the defined render target or the main framebuffer.
 * 
 * @note This function must be called after all `R3D_Draw` operations for a frame are completed.
 */
void R3D_End(void);

/**
 * @brief Retrieves the default black texture used by the renderer.
 * 
 * This function returns a pointer to a predefined black texture. This texture is commonly used as 
 * a placeholder or default value for material maps, such as normal maps or emission maps, when no 
 * specific texture is assigned.
 * 
 * @return A constant pointer to the default black `Texture2D`.
 * 
 * @note The returned texture is managed internally by R3D and does not require explicit destruction.
 */
const Texture2D* R3D_GetDefaultTextureBlack(void);

/**
 * @brief Retrieves the default white texture used by the renderer.
 * 
 * This function returns a pointer to a predefined white texture. This texture is often used as a 
 * default value for material maps, such as albedo maps or roughness maps, when no specific texture is assigned.
 * 
 * @return A constant pointer to the default white `Texture2D`.
 * 
 * @note The returned texture is managed internally by R3D and does not require explicit destruction.
 */
const Texture2D* R3D_GetDefaultTextureWhite(void);


/* [Core] Environment Functions */

/**
 * @brief Retrieves the current environment settings used by the renderer.
 * 
 * This function returns a pointer to the current `R3D_Environment` structure, which contains
 * all global rendering settings, including bloom, fog, tonemapping, and more.
 * 
 * @return A pointer to the current `R3D_Environment`.
 * 
 * @note Modifying the returned environment directly may lead to undefined behavior.
 *       Use `R3D_SetEnvironment` to apply changes safely.
 */
R3D_Environment* R3D_GetEnvironment(void);

/**
 * @brief Sets the current environment settings for the renderer.
 * 
 * This function replaces the current `R3D_Environment` settings with the provided structure.
 * Use this to configure global rendering options, such as bloom effects, fog, and tonemapping.
 * 
 * @param env A pointer to the new `R3D_Environment` structure to apply.
 * 
 * @note The `env` parameter must remain valid while in use by the renderer.
 */
void R3D_SetEnvironment(const R3D_Environment* env);

/**
 * @brief Retrieves the current bloom mode in the environment settings.
 * 
 * This function returns the active bloom mode, which determines how bloom is applied
 * to the rendered scene.
 * 
 * @return The current `R3D_Bloom` mode. Default is `R3D_BLOOM_DISABLED`.
 */
R3D_Bloom R3D_GetEnvBloomMode(void);

/**
 * @brief Sets the bloom mode in the environment settings.
 * 
 * This function configures the type of bloom effect to be applied during rendering.
 * 
 * @param mode The desired `R3D_Bloom` mode. Use `R3D_BLOOM_DISABLED` to disable bloom.
 */
void R3D_SetEnvBloomMode(R3D_Bloom mode);

/**
 * @brief Retrieves the bloom intensity multiplier.
 * 
 * This value determines the strength of the bloom effect by applying a multiplier to
 * the values extracted from the HDR buffer during the blur phase.
 * 
 * @return The current bloom intensity. Default is `1.0f`.
 */
float R3D_GetEnvBloomIntensity(void);

/**
 * @brief Sets the bloom intensity multiplier.
 * 
 * This function adjusts the strength of the bloom effect by modifying the multiplier applied
 * to HDR values during the blur phase.
 * 
 * @param intensity The desired bloom intensity multiplier. Must be non-negative.
 */
void R3D_SetEnvBloomIntensity(float intensity);

/**
 * @brief Retrieves the HDR threshold for the bloom effect.
 * 
 * This value specifies the minimum luminance required for a pixel to be written to
 * the bloom blur buffer.
 * 
 * @return The current HDR threshold for bloom. Default is `1.0f`.
 */
float R3D_GetEnvBloomHDRThreshold(void);

/**
 * @brief Sets the HDR threshold for the bloom effect.
 * 
 * This function defines the luminance threshold at which pixels contribute to the bloom effect.
 * 
 * @param threshold The desired HDR threshold. Must be non-negative.
 */
void R3D_SetEnvBloomHDRThreshold(float threshold);

/**
 * @brief Retrieves the number of blur iterations for the bloom effect.
 * 
 * The number of iterations controls the quality and spread of the Gaussian blur
 * applied during the bloom effect.
 * 
 * @return The current number of blur iterations. Default is `10`.
 */
int R3D_GetEnvBloomIterations(void);

/**
 * @brief Sets the number of blur iterations for the bloom effect.
 * 
 * This function adjusts the quality and spread of the Gaussian blur used in the bloom effect.
 * 
 * @param iterations The desired number of blur iterations. Must be a non-negative integer.
 */
void R3D_SetEnvBloomIterations(int iterations);

/**
 * @brief Retrieves the current fog mode in the environment settings.
 * 
 * The fog mode determines how the fog effect is applied to the scene. By default, fog is disabled.
 * 
 * @return The current `R3D_Fog` mode. Default is `R3D_FOG_DISABLED`.
 */
R3D_Fog R3D_GetEnvFogMode(void);

/**
 * @brief Sets the fog mode in the environment settings.
 * 
 * This function configures the type of fog effect to be applied during rendering.
 * 
 * @param mode The desired `R3D_Fog` mode. Use `R3D_FOG_DISABLED` to disable fog.
 */
void R3D_SetEnvFogMode(R3D_Fog mode);

/**
 * @brief Retrieves the current fog color.
 * 
 * The fog color determines the color used for the fog effect, blending with the scene
 * based on the selected fog mode.
 * 
 * @return The current `Color` used for fog. Default is `GRAY`.
 */
Color R3D_GetEnvFogColor(void);

/**
 * @brief Sets the fog color in the environment settings.
 * 
 * This function specifies the color to be used for the fog effect during rendering.
 * 
 * @param color The desired `Color` for the fog.
 */
void R3D_SetEnvFogColor(Color color);

/**
 * @brief Retrieves the start distance for linear fog.
 * 
 * This value determines the distance from the camera at which linear fog starts to appear.
 * 
 * @return The current start distance for linear fog. Default is `10.0f`.
 * 
 * @note This value is only used with the `R3D_FOG_LINEAR` mode.
 */
float R3D_GetEnvFogStart(void);

/**
 * @brief Sets the start distance for linear fog.
 * 
 * This function configures the distance from the camera at which linear fog begins to blend with the scene.
 * 
 * @param start The desired start distance for linear fog. Must be non-negative.
 * 
 * @note This value is only used with the `R3D_FOG_LINEAR` mode.
 */
void R3D_SetEnvFogStart(float start);

/**
 * @brief Retrieves the end distance for linear fog.
 * 
 * This value determines the distance from the camera at which linear fog completely blends with the scene.
 * 
 * @return The current end distance for linear fog. Default is `30.0f`.
 * 
 * @note This value is only used with the `R3D_FOG_LINEAR` mode.
 */
float R3D_GetEnvFogEnd(void);

/**
 * @brief Sets the end distance for linear fog.
 * 
 * This function specifies the distance from the camera at which linear fog fully blends with the scene.
 * 
 * @param end The desired end distance for linear fog. Must be greater than the start distance.
 * 
 * @note This value is only used with the `R3D_FOG_LINEAR` mode.
 */
void R3D_SetEnvFogEnd(float end);

/**
 * @brief Retrieves the density value for exponential fog.
 * 
 * The density value controls the intensity of the fog in exponential modes (`R3D_FOG_EXP` and `R3D_FOG_EXP2`).
 * 
 * @return The current fog density. Default is `0.1f`.
 * 
 * @note This value is only used with `R3D_FOG_EXP` and `R3D_FOG_EXP2` modes.
 */
float R3D_GetEnvFogDensity(void);

/**
 * @brief Sets the density value for exponential fog.
 * 
 * This function adjusts the intensity of the fog effect in exponential fog modes.
 * 
 * @param density The desired fog density. Must be non-negative.
 * 
 * @note This value is only used with `R3D_FOG_EXP` and `R3D_FOG_EXP2` modes.
 */
void R3D_SetEnvFogDensity(float density);

/**
 * @brief Retrieves the current tonemap mode in the environment settings.
 * 
 * The tonemap mode determines how HDR (High Dynamic Range) values are mapped to the display range.
 * By default, the tonemap mode is set to `R3D_TONEMAP_LINEAR`.
 * 
 * @return The current `R3D_Tonemap` mode. Default is `R3D_TONEMAP_LINEAR`.
 */
R3D_Tonemap R3D_GetEnvTonemapMode(void);

/**
 * @brief Sets the tonemap mode in the environment settings.
 * 
 * This function configures how HDR values will be tonemapped to fit within the displayable range.
 * 
 * @param mode The desired `R3D_Tonemap` mode. Possible values are `R3D_TONEMAP_LINEAR`, `R3D_TONEMAP_REINHARD`, `R3D_TONEMAP_FILMIC`, and `R3D_TONEMAP_ACES`.
 */
void R3D_SetEnvTonemapMode(R3D_Tonemap mode);

/**
 * @brief Retrieves the current exposure value for the tonemapping in the environment settings.
 * 
 * Exposure adjusts the overall brightness of the scene, simulating the camera's exposure settings.
 * 
 * @return The current exposure value. Default is `1.0f`.
 */
float R3D_GetEnvTonemapExposure(void);

/**
 * @brief Sets the exposure value for the tonemapping in the environment settings.
 * 
 * Exposure modifies the overall brightness of the scene, allowing you to simulate overexposure or underexposure.
 * 
 * @param exposure The desired exposure value. Typically set between `0.1f` and `10.0f`.
 */
void R3D_SetEnvTonemapExposure(float exposure);

/**
 * @brief Sets the white point value for the tonemapping in the environment settings.
 * 
 * This function allows setting the white point (white reference) for tonemapping, which influences how bright highlights are displayed.
 * 
 * @param white The desired white point value. This is not used with the `R3D_TONEMAP_LINEAR` mode.
 */
void R3D_SetEnvTonemapWhite(float white);

/**
 * @brief Retrieves the current brightness adjustment value for the environment.
 * 
 * Brightness adjusts the overall brightness of the scene. A value of `1.0f` means no adjustment.
 * 
 * @return The current brightness value. Default is `1.0f`.
 */
float R3D_GetEnvAdjustBrightness(void);

/**
 * @brief Sets the brightness adjustment value for the environment.
 * 
 * This function controls the overall brightness of the scene.
 * 
 * @param brightness The desired brightness value. Values greater than `1.0f` will brighten the scene, while values less than `1.0f` will darken it.
 */
void R3D_SetEnvAdjustBrightness(float brightness);

/**
 * @brief Retrieves the current contrast adjustment value for the environment.
 * 
 * Contrast adjusts the difference between light and dark areas in the scene. A value of `1.0f` means no contrast adjustment.
 * 
 * @return The current contrast value. Default is `1.0f`.
 */
float R3D_GetEnvAdjustContrast(void);

/**
 * @brief Sets the contrast adjustment value for the environment.
 * 
 * This function adjusts the contrast of the scene. A higher contrast value will increase the difference between light and dark areas.
 * 
 * @param contrast The desired contrast value. Typically set between `0.0f` and `2.0f`.
 */
void R3D_SetEnvAdjustContrast(float contrast);

/**
 * @brief Retrieves the current saturation adjustment value for the environment.
 * 
 * Saturation adjusts the intensity of colors in the scene. A value of `1.0f` means no adjustment.
 * 
 * @return The current saturation value. Default is `1.0f`.
 */
float R3D_GetEnvAdjustSaturation(void);

/**
 * @brief Sets the saturation adjustment value for the environment.
 * 
 * This function controls the color saturation of the scene. A higher value will make colors more intense, while a lower value will desaturate them.
 * 
 * @param saturation The desired saturation value. Values less than `1.0f` will desaturate the scene, while values greater than `1.0f` will increase saturation.
 */
void R3D_SetEnvAdjustSaturation(float saturation);

/**
 * @brief Retrieves the current skybox used in the environment settings.
 * 
 * A skybox is a background texture or environment map used to simulate distant backgrounds such as the sky. If no skybox is set, it will return `NULL`.
 * When a skybox is used, it replaces the ambient color applied to the scene via the skybox's irradiance map.
 * 
 * @return The current `R3D_Skybox` in the environment. If no skybox is set, `NULL` is returned.
 */
R3D_Skybox R3D_GetEnvWorldSkybox(void);

/**
 * @brief Sets the skybox used in the environment settings.
 * 
 * This function allows you to set a skybox that will be used as the background for the scene. 
 * If `NULL` is passed, the skybox will be disabled, and the default background color and ambient light will be used instead.
 * 
 * @param skybox The `R3D_Skybox` to set as the world background. Passing `NULL` will disable the skybox.
 */
void R3D_SetEnvWorldSkybox(R3D_Skybox skybox);

/**
 * @brief Retrieves the current ambient color for the world when no skybox is used.
 * 
 * The ambient color is used to apply a global light to the scene in the absence of a skybox. If a skybox is being used, the irradiance map from the skybox will replace this color.
 * 
 * @return The current ambient color (`Color`). Default is typically `DARKGRAY` or as defined by the environment settings.
 */
Color R3D_GetEnvWorldAmbient(void);

/**
 * @brief Sets the ambient color for the world when no skybox is used.
 * 
 * This color is applied to all surfaces in the scene when there is no skybox, or if the material of a surface does not use IBL (Image-Based Lighting) with the skybox.
 * 
 * @param color The `Color` to be set as the world ambient color. This is only applied if no skybox is used or if the material does not have the `R3D_MATERIAL_FLAG_SKY_IBL` flag.
 */
void R3D_SetEnvWorldAmbient(Color color);

/**
 * @brief Retrieves the current background color used when no skybox is used.
 * 
 * The background color is used when there is no skybox, and it is the color that will be used to clear the framebuffer when rendering.
 * 
 * @return The current background color (`Color`). Default is typically `GRAY` or as defined by the environment settings.
 */
Color R3D_GetEnvWorldBackground(void);

/**
 * @brief Sets the background color used when no skybox is used.
 * 
 * This color is used when there is no skybox, and it is the color that will be used to clear the framebuffer in the internal rendering targets.
 * 
 * @param color The `Color` to set as the background color. This will be used when no skybox is available.
 */
void R3D_SetEnvWorldBackground(Color color);


/* [Core] Material Functions */

/**
 * @brief Creates a new material configuration with the specified parameters.
 * 
 * This function helps to create a material configuration (`R3D_MaterialConfig`) by specifying the diffuse and specular modes, the blending mode, 
 * culling mode, and other flags related to the material. It is highly recommended to use this function to create material configurations to avoid 
 * the creation of multiple batch or shaders for the same material.
 * 
 * The `flags` parameter should be a combination of the `R3D_MaterialFlags`. Unused reserved fields should be set to `0` to prevent potential issues.
 * If necessary, the function automatically calls `R3D_RegisterMaterialConfig` to compile the shaders.
 * 
 * @param diffuse The diffuse shading mode (`R3D_DiffuseMode`).
 * @param specular The specular shading mode (`R3D_SpecularMode`).
 * @param blendMode The blending mode (`R3D_BlendMode`).
 * @param cullMode The culling mode (`R3D_CullMode`).
 * @param flags The material flags (`R3D_MaterialFlags`).
 * 
 * @return A `R3D_MaterialConfig` structure with the specified settings.
 */
R3D_MaterialConfig R3D_CreateMaterialConfig(R3D_DiffuseMode diffuse, R3D_SpecularMode specular,
                                            R3D_BlendMode blendMode, R3D_CullMode cullMode,
                                            int flags);

/**
 * @brief Creates a new material from the specified configuration.
 * 
 * This function creates a new material using a provided `R3D_MaterialConfig`. The material configuration should be valid; it is recommended 
 * to create the configuration using `R3D_CreateMaterialConfig` to ensure it is properly set up.
 * 
 * @param config The `R3D_MaterialConfig` to be used for the material.
 * 
 * @return A `R3D_Material` object created from the provided configuration.
 */
R3D_Material R3D_CreateMaterial(R3D_MaterialConfig config);

/**
 * @brief Retrieves the default material configuration.
 * 
 * This function returns the default material configuration. It typically includes default settings for diffuse and specular modes, 
 * blending, culling, and other material properties.
 * 
 * @return The default `R3D_MaterialConfig`.
 */
R3D_MaterialConfig R3D_GetDefaultMaterialConfig(void);

/**
 * @brief Sets the default material configuration.
 * 
 * This function sets the default material configuration. The provided configuration will be used as the default material settings for new materials.
 * 
 * It also automatically calls `R3D_RegisterMaterialConfig` for the given configuration to ensure shaders are compiled if needed.
 * 
 * @param config The `R3D_MaterialConfig` to be set as the default.
 */
void R3D_SetDefaultMaterialConfig(R3D_MaterialConfig config);

/**
 * @brief Registers a material configuration and compiles the shader if needed.
 * 
 * This function registers a material configuration (`R3D_MaterialConfig`) and compiles the corresponding shader if necessary.
 * It is generally called automatically when creating or loading a material configuration via functions like `R3D_CreateMaterialConfig`.
 * 
 * You can call this function directly for more control over shader compilation, for example, at the start of the program to avoid shader compilation
 * during the main loop execution.
 * 
 * @param config The material configuration.
 */
void R3D_RegisterMaterialConfig(R3D_MaterialConfig config);

/**
 * @brief Releases a material configuration, use with caution.
 * 
 * This function releases a material configuration. It is typically not necessary to call this at the program's shutdown, as the shaders used internally
 * are automatically destroyed when `R3D_Close` is called.
 * 
 * @param config The material configuration to be unloaded.
 */
void R3D_UnloadMaterialConfig(R3D_MaterialConfig config);

/**
 * @brief Checks whether a material configuration has a valid shader.
 * 
 * This function returns `false` if the provided configuration does not have a valid shader associated with it. In such a case, 
 * you should call `R3D_RegisterMaterialConfig` to compile the shader.
 * 
 * @param config The material configuration to check.
 * @return `true` if the configuration has a valid shader, `false` otherwise.
 */
bool R3D_IsMaterialConfigValid(R3D_MaterialConfig config);


/* [Core] Lighting functions */

/**
 * @brief Creates a light of the specified type.
 * 
 * This function creates a light of the specified type and sets its shadow map resolution. If you pass `0` as the shadowMapResolution,
 * the light will not cast shadows.
 * 
 * By default, the light is created in a disabled state (i.e., not active).
 * 
 * @param type The type of light to create (e.g., directional, spotlight, or omnidirectional light).
 * @param shadowMapResolution The resolution of the shadow map. Pass `0` to disable shadows for this light.
 * @return A handle to the created light.
 */
R3D_Light R3D_CreateLight(R3D_LightType type, int shadowMapResolution);

/**
 * @brief Destroys a light.
 * 
 * This function destroys the specified light. The light ID (`R3D_Light`) will be reused for the next light created.
 * 
 * @warning: If the light casts shadows, the associated shadow map will be destroyed as well. This operation can be costly.
 * It is generally better to disable the light instead of destroying it unless you are sure about what you're doing.
 * 
 * @param light The light handle to be destroyed.
 */
void R3D_DestroyLight(R3D_Light light);

/**
 * @brief Checks if the light is active.
 * 
 * This function checks if the specified light is currently active.
 * 
 * @param light The light handle to check.
 * @return `true` if the light is active, `false` otherwise.
 */
bool R3D_IsLightActive(R3D_Light light);

/**
 * @brief Sets the light as active or inactive.
 * 
 * This function allows you to set the light to be either active or inactive.
 * 
 * @param light The light handle to modify.
 * @param enabled `true` to enable the light, `false` to disable it.
 */
void R3D_SetLightActive(R3D_Light light, bool enabled);

/**
 * @brief Toggles the light state (active/inactive).
 * 
 * This function toggles the light's current state. If the light is active, it will be deactivated, and if it is inactive, it will be activated.
 * 
 * @param light The light handle to toggle.
 */
void R3D_ToggleLight(R3D_Light light);

/**
 * @brief Gets the color of the light.
 * 
 * This function retrieves the current color of the specified light.
 * 
 * @param light The light handle from which to retrieve the color.
 * @return The current color of the light.
 */
Color R3D_GetLightColor(R3D_Light light);

/**
 * @brief Sets the color of the light.
 * 
 * This function sets the color of the specified light.
 * 
 * @param light The light handle to modify.
 * @param color The new color to assign to the light.
 */
void R3D_SetLightColor(R3D_Light light, Color color);

/**
 * @brief Gets the position of the light.
 * 
 * This function retrieves the current position of the specified light, if applicable (for position-based lights).
 * 
 * @param light The light handle from which to retrieve the position.
 * @return The current position of the light.
 */
Vector3 R3D_GetLightPosition(R3D_Light light);

/**
 * @brief Sets the position of the light.
 * 
 * This function sets the position of the specified light (for position-based lights). 
 * If shadows are enabled for this light, the frustum is recalculated based on the new position, 
 * which incurs some performance cost. It is recommended to use `R3D_SetLightPositionTarget` for 
 * lights producing shadows, as it allows setting both the position and target while recalculating 
 * the frustum in one operation.
 * 
 * @param light The light handle to modify.
 * @param position The new position to assign to the light.
 */
void R3D_SetLightPosition(R3D_Light light, Vector3 position);

/**
 * @brief Gets the direction of the light.
 * 
 * This function retrieves the current direction of the specified light (for directional or spotlight lights).
 * 
 * @param light The light handle from which to retrieve the direction.
 * @return The current direction of the light.
 */
Vector3 R3D_GetLightDirection(R3D_Light light);

/**
 * @brief Sets the direction of the light.
 * 
 * This function sets the direction of the specified light (for directional or spotlight lights). 
 * If shadows are enabled for this light, the frustum is recalculated based on the new direction, 
 * which incurs some performance cost. It is recommended to use `R3D_SetLightPositionTarget` for 
 * lights producing shadows, as it allows setting both the position and direction while recalculating 
 * the frustum in one operation.
 * 
 * @param light The light handle to modify.
 * @param direction The new direction to assign to the light.
 */
void R3D_SetLightDirection(R3D_Light light, Vector3 direction);

/**
 * @brief Sets the target of the light.
 * 
 * This function sets the target point to which the light is directed. The target is not constant 
 * because lights are based on their directions. If shadows are enabled for this light, the frustum 
 * is recalculated based on the new target, which incurs some performance cost. It is recommended to 
 * use `R3D_SetLightPositionTarget` for lights producing shadows, as it allows setting both the position 
 * and target while recalculating the frustum in one operation.
 * 
 * @param light The light handle to modify.
 * @param target The new target point the light will point towards.
 */
void R3D_SetLightTarget(R3D_Light light, Vector3 target);

/**
 * @brief Sets the position and target of the light in one operation.
 * 
 * This function sets both the position and the target point for the light. This is particularly 
 * useful for lights that produce shadows, as it recalculates the frustum based on both the 
 * position and the target in a single call, thus avoiding the performance overhead of recalculating 
 * the frustum separately for each property.
 * 
 * It is recommended to use this function for lights producing shadows, as it optimizes the process 
 * of setting the light's position and direction (via the target) while recalculating the frustum only once.
 * 
 * @param light The light handle to modify.
 * @param position The new position to assign to the light.
 * @param target The new target point the light will point towards.
 */
void R3D_SetLightPositionTarget(R3D_Light light, Vector3 position, Vector3 target);

/**
 * @brief Gets the energy of the light.
 * 
 * This function retrieves the current energy (brightness) value of the specified light.
 * 
 * @param light The light handle from which to retrieve the energy.
 * @return The current energy of the light.
 */
float R3D_GetLightEnergy(R3D_Light light);

/**
 * @brief Sets the energy of the light.
 * 
 * This function sets the energy (brightness) of the specified light.
 * 
 * @param light The light handle to modify.
 * @param energy The new energy value to assign to the light.
 */
void R3D_SetLightEnergy(R3D_Light light, float energy);

/**
 * @brief Gets the range (distance) of the light.
 * 
 * This function retrieves the current range (distance) value of the specified light. 
 * The range affects how far the light illuminates.
 * 
 * @param light The light handle from which to retrieve the range.
 * @return The current range of the light in units.
 */
float R3D_GetLightRange(R3D_Light light);

/**
 * @brief Sets the range (distance) of the light.
 * 
 * This function sets the range (distance) of the specified light. 
 * The range determines how far the light can illuminate.
 * 
 * @param light The light handle to modify.
 * @param distance The new range value to assign to the light in units.
 */
void R3D_SetLightRange(R3D_Light light, float distance);

/**
 * @brief Gets the attenuation factor of the light.
 * 
 * This function retrieves the current attenuation factor of the specified light. 
 * The attenuation factor controls how quickly the light diminishes over distance.
 * 
 * @param light The light handle from which to retrieve the attenuation factor.
 * @return The current attenuation factor of the light.
 */
float R3D_GetLightAttenuation(R3D_Light light);

/**
 * @brief Sets the attenuation factor of the light.
 * 
 * This function sets the attenuation factor of the specified light. 
 * The attenuation factor determines how quickly the light intensity decreases with distance.
 * 
 * @param light The light handle to modify.
 * @param factor The new attenuation factor to assign to the light.
 */
void R3D_SetLightAttenuation(R3D_Light light, float factor);

/**
 * @brief Gets the inner cutoff angle of the light (for spotlight type lights).
 * 
 * This function retrieves the current inner cutoff angle of the specified spotlight. 
 * The inner cutoff angle determines where the spotlight effect begins. The angle is returned in degrees.
 * 
 * @param light The light handle from which to retrieve the inner cutoff angle.
 * @return The current inner cutoff angle in degrees.
 */
float R3D_GetLightInnerCutOff(R3D_Light light);

/**
 * @brief Sets the inner cutoff angle of the light (for spotlight type lights).
 * 
 * This function sets the inner cutoff angle of the specified spotlight. 
 * The inner cutoff angle determines where the spotlight effect begins. The angle should be provided in degrees.
 * 
 * @param light The light handle to modify.
 * @param angle The new inner cutoff angle to assign to the light in degrees.
 */
void R3D_SetLightInnerCutOff(R3D_Light light, float angle);

/**
 * @brief Gets the outer cutoff angle of the light (for spotlight type lights).
 * 
 * This function retrieves the current outer cutoff angle of the specified spotlight. 
 * The outer cutoff angle determines the point at which the spotlight effect ends. The angle is returned in degrees.
 * 
 * @param light The light handle from which to retrieve the outer cutoff angle.
 * @return The current outer cutoff angle in degrees.
 */
float R3D_GetLightOuterCutOff(R3D_Light light);

/**
 * @brief Sets the outer cutoff angle of the light (for spotlight type lights).
 * 
 * This function sets the outer cutoff angle of the specified spotlight. 
 * The outer cutoff angle determines the point at which the spotlight effect ends. The angle should be provided in degrees.
 * 
 * @param light The light handle to modify.
 * @param angle The new outer cutoff angle to assign to the light in degrees.
 */
void R3D_SetLightOuterCutOff(R3D_Light light, float angle);

/**
 * @brief Gets the shadow bias for the light.
 * 
 * This function retrieves the current shadow bias value for the specified light. 
 * The shadow bias is used to prevent shadow acne (artifacts) by adjusting the depth comparison threshold. 
 * A higher value can reduce shadow artifacts but may cause shadows to appear detached from objects.
 * 
 * @param light The light handle from which to retrieve the shadow bias.
 * @return The current shadow bias value.
 */
float R3D_GetLightShadowBias(R3D_Light light);

/**
 * @brief Sets the shadow bias for the light.
 * 
 * This function sets the shadow bias value for the specified light. 
 * The shadow bias helps reduce shadow artifacts like shadow acne by adjusting the depth comparison threshold.
 * 
 * @param light The light handle to modify.
 * @param bias The new shadow bias value to assign to the light.
 */
void R3D_SetLightShadowBias(R3D_Light light, float bias);

/**
 * @brief Checks if the light produces shadows.
 * 
 * This function returns whether the specified light is configured to produce shadows.
 * 
 * @param light The light handle to check.
 * @return True if the light produces shadows, false otherwise.
 */
bool R3D_IsLightProduceShadows(R3D_Light light);

/**
 * @brief Enables shadows for the light and creates the shadow map.
 * 
 * This function enables shadows for the specified light. If shadows are not already enabled or if the resolution is 
 * less than or equal to zero, the function creates a shadow map for the light with the specified resolution.
 * 
 * @param light The light handle to modify.
 * @param shadowMapResolution The resolution of the shadow map in pixels.
 * 
 * @warning This operation creates the shadow map, which may be costly. It does nothing if shadows are already enabled 
 * or if the specified resolution is less than or equal to zero.
 */
void R3D_EnableLightShadow(R3D_Light light, int shadowMapResolution);

/**
 * @brief Disables shadows for the light and destroys the shadow map.
 * 
 * This function disables shadows for the specified light. It will destroy the shadow map associated with the light, 
 * which may be a costly operation.
 * 
 * @param light The light handle to modify.
 * 
 * @warning This operation destroys the shadow map, which can be costly. It does nothing if shadows are already inactive.
 */
void R3D_DisableLightShadow(R3D_Light light);

/**
 * @brief Gets the type of the light.
 * 
 * This function retrieves the current type of the specified light. The light type defines how the light behaves and 
 * how it affects the scene.
 * 
 * @param light The light handle from which to retrieve the light type.
 * @return The current type of the light.
 */
R3D_LightType R3D_GetLightType(R3D_Light light);

/**
 * @brief Sets the type of the light.
 * 
 * This function sets the type of the specified light. Changing the light type can alter its behavior, such as whether it 
 * behaves as a point light, spotlight, or directional light.
 * 
 * @param light The light handle to modify.
 * @param type The new light type to assign to the light.
 * 
 * @warning Calling this function can cause a reallocation of the shadow map if shadows are enabled.
 */
void R3D_SetLightType(R3D_Light light, R3D_LightType type);


/* [Core] Debug functions */

/**
 * @brief Retrieves the count of draw calls for the scene and shadows.
 * 
 * This function returns the number of elements that will be drawn during the `R3D_End` function call, which 
 * is between `R3D_Begin` and `R3D_End`. It does not store values directly, instead, it counts the number of 
 * draw calls for scene rendering and shadow rendering that are scheduled to be drawn.
 * 
 * The `sceneDrawCount` and `shadowDrawCount` provide the count of draw calls for the scene and shadow elements, respectively.
 * 
 * @param sceneDrawCount A pointer to an integer to store the count of scene draw calls.
 * @param shadowDrawCount A pointer to an integer to store the count of shadow draw calls.
 * 
 * @note It's safe to pass `NULL` to these parameters if you do not want to retrieve the count for a particular type of draw call.
 *       If the corresponding pointer is `NULL`, that draw call type will not be counted.
 */
void R3D_GetDrawCallCount(int* sceneDrawCount, int* shadowDrawCount);

/**
 * @brief Draws the shadow map for debugging purposes.
 * 
 * This function is used to visualize the shadow map produced by a light source for debugging purposes.
 * To use this feature, ensure that the flag `R3D_FLAG_DEBUG_SHADOW_MAP` is set during the R3D initialization.
 * 
 * The shadow map visualization will be drawn at the specified position and dimensions on the screen.
 * 
 * @param light The light whose shadow map will be drawn.
 * @param x The x-coordinate where the shadow map will be drawn on the screen.
 * @param y The y-coordinate where the shadow map will be drawn on the screen.
 * @param width The width of the shadow map to be drawn.
 * @param height The height of the shadow map to be drawn.
 * @param zNear The near clipping plane distance for the shadow map.
 * @param zFar The far clipping plane distance for the shadow map.
 * 
 * @note Ensure that the flag `R3D_FLAG_DEBUG_SHADOW_MAP` is set at initialization to enable this feature.
 */
void R3D_DrawShadowMap(R3D_Light light, int x, int y, int width, int height, float zNear, float zFar);


/* Skybox Functions */

/**
 * @brief Loads a skybox from a set of textures.
 * 
 * This function loads a skybox from the specified texture file, arranged in the given cubemap layout.
 * The textures will be loaded into the appropriate faces of the cubemap to create the skybox.
 * 
 * @param fileName The path to the texture file to load.
 * @param layout The cubemap layout that determines how the textures are arranged (e.g., horizontal or vertical layout).
 * 
 * @return A `R3D_Skybox` object that represents the loaded skybox.
 */
R3D_Skybox R3D_LoadSkybox(const char* fileName, CubemapLayout layout);

/**
 * @brief Loads a high dynamic range (HDR) skybox from a single file.
 * 
 * This function loads an HDR skybox from a specified file. It is typically used for more realistic skyboxes 
 * that provide better lighting information for global illumination or background reflections.
 * 
 * @param fileName The path to the HDR image file to load.
 * @param sizeFace The size of each face of the cubemap. This is usually the resolution of the HDR image.
 * 
 * @return A `R3D_Skybox` object representing the loaded HDR skybox.
 */
R3D_Skybox R3D_LoadSkyboxHDR(const char* fileName, int sizeFace);

/**
 * @brief Unloads a skybox and frees its resources.
 * 
 * This function unloads the specified skybox from memory. It is safe to call this function even if the skybox 
 * is currently in use; in that case, the skybox being unloaded will be replaced with a `NULL` skybox, and no skybox 
 * will be used during rendering.
 * 
 * @param skybox The `R3D_Skybox` object to unload.
 * 
 * @note It is safe to provide a skybox that is currently being used in the scene. If so, the skybox will be replaced with `NULL`.
 */
void R3D_UnloadSkybox(R3D_Skybox skybox);


/* Material Functions */

/**
 * @brief Loads a model from a file.
 * 
 * This function loads a model from the specified file. The model will have its default material applied to each surface.
 * 
 * @param fileName The path to the model file to load.
 * 
 * @return A `R3D_Model` object representing the loaded model.
 */
R3D_Model R3D_LoadModel(const char* fileName);

/**
 * @brief Loads a model from a mesh.
 * 
 * This function creates a model from the provided `Mesh`. The model surfaces will have the default material applied.
 * 
 * @param mesh The `Mesh` object to load as a model.
 * 
 * @return A `R3D_Model` object representing the model created from the mesh.
 */
R3D_Model R3D_LoadModelFromMesh(Mesh mesh);

/**
 * @brief Unloads a model and frees its resources.
 * 
 * This function unloads the specified model from memory. It is recommended to call this function when the model is no longer needed 
 * to release its associated resources.
 * 
 * @param model A pointer to the `R3D_Model` object to unload.
 */
void R3D_UnloadModel(R3D_Model* model);

/**
 * @brief Gets the number of surfaces in the model.
 * 
 * This function returns the number of surfaces present in the model. Each surface may have a different material and mesh associated with it.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * 
 * @return The number of surfaces in the model.
 */
int R3D_GetSurfaceCount(const R3D_Model* model);

/**
 * @brief Gets a specific surface of the model.
 * 
 * This function returns the surface at the given index in the model. A surface consists of a mesh and material that are rendered together.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to retrieve.
 * 
 * @return A pointer to the `R3D_Surface` object corresponding to the surface at the specified index.
 */
R3D_Surface* R3D_GetSurface(R3D_Model* model, int surfaceIndex);

/**
 * @brief Gets the mesh of a specific surface in the model.
 * 
 * This function returns the `Mesh` object of the surface at the given index in the model. The mesh defines the geometry for rendering.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface whose mesh is to be retrieved.
 * 
 * @return A pointer to the `Mesh` object corresponding to the specified surface.
 */
Mesh* R3D_GetMesh(R3D_Model* model, int surfaceIndex);

/**
 * @brief Gets the material of a specific surface in the model.
 * 
 * This function returns the material applied to the surface at the specified index in the model. The material defines how the surface
 * will be rendered (e.g., its color, texture, and shading properties).
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface whose material is to be retrieved.
 * 
 * @return A pointer to the `R3D_Material` object corresponding to the material of the specified surface.
 */
R3D_Material* R3D_GetMaterial(R3D_Model* model, int surfaceIndex);

/**
 * @brief Sets the material of a specific surface in the model.
 * 
 * This function applies a new material to the surface at the given index in the model. The material will define how the surface
 * is rendered.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the material should be applied.
 * @param material A pointer to the `R3D_Material` object to assign to the surface.
 */
void R3D_SetMaterial(R3D_Model* model, int surfaceIndex, const R3D_Material* material);

/**
 * @brief Gets the material configuration of a specific surface in the model.
 * 
 * This function returns the material configuration of a surface at the given index in the model. The configuration includes
 * properties such as diffuse mode, specular mode, and other settings.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface whose material configuration is to be retrieved.
 * 
 * @return The `R3D_MaterialConfig` object representing the material configuration of the specified surface.
 */
R3D_MaterialConfig R3D_GetMaterialConfig(R3D_Model* model, int surfaceIndex);

/**
 * @brief Sets the material configuration of a specific surface in the model.
 * 
 * This function applies a new material configuration to the surface at the given index in the model. The configuration includes
 * properties such as diffuse mode, specular mode, and other settings.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the material configuration should be applied.
 * @param config The `R3D_MaterialConfig` object containing the new material configuration.
 */
void R3D_SetMaterialConfig(R3D_Model* model, int surfaceIndex, R3D_MaterialConfig config);

/**
 * @brief Sets the albedo map (diffuse texture) for a specific surface in the model.
 * 
 * This function assigns a texture to be used as the albedo (diffuse) map for the surface at the given index in the model.
 * If `NULL` is provided for the texture, the previous texture or the default texture will be preserved.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the albedo map should be applied.
 * @param texture A pointer to the `Texture2D` object to be used as the albedo map, or `NULL` to preserve the previous texture.
 * @param color The color applied to the albedo map. This is used if the texture is `NULL`.
 */
void R3D_SetMapAlbedo(R3D_Model* model, int surfaceIndex, const Texture2D* texture, Color color);

/**
 * @brief Sets the metalness map for a specific surface in the model.
 * 
 * This function assigns a texture to be used as the metalness map for the surface at the given index in the model.
 * If `NULL` is provided for the texture, the previous texture or the default texture will be preserved.
 * The `factor` parameter controls the intensity of the metalness effect.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the metalness map should be applied.
 * @param texture A pointer to the `Texture2D` object to be used as the metalness map, or `NULL` to preserve the previous texture.
 * @param factor A float value controlling the intensity of the metalness effect.
 */
void R3D_SetMapMetalness(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float factor);

/**
 * @brief Sets the roughness map for a specific surface in the model.
 * 
 * This function assigns a texture to be used as the roughness map for the surface at the given index in the model.
 * If `NULL` is provided for the texture, the previous texture or the default texture will be preserved.
 * The `factor` parameter controls the intensity of the roughness effect.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the roughness map should be applied.
 * @param texture A pointer to the `Texture2D` object to be used as the roughness map, or `NULL` to preserve the previous texture.
 * @param factor A float value controlling the intensity of the roughness effect.
 */
void R3D_SetMapRoughness(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float factor);

/**
 * @brief Sets the emission map for a specific surface in the model.
 * 
 * This function assigns a texture to be used as the emission map for the surface at the given index in the model.
 * If `NULL` is provided for the texture, the previous texture or the default texture will be preserved.
 * The `energy` parameter controls the intensity of the emission effect, and the `color` parameter defines the emission color.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the emission map should be applied.
 * @param texture A pointer to the `Texture2D` object to be used as the emission map, or `NULL` to preserve the previous texture.
 * @param energy A float value controlling the intensity of the emission effect.
 * @param color The color of the emission.
 */
void R3D_SetMapEmission(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float energy, Color color);

/**
 * @brief Sets the normal map for a specific surface in the model.
 * 
 * This function assigns a normal map texture to a specific surface of the model. The normal map defines the surface's 
 * details for lighting calculations, simulating bumps and wrinkles. If `NULL` is provided for the texture, the previous 
 * texture or the default one will be preserved.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the normal map should be applied.
 * @param texture A pointer to the `Texture2D` object to be used as the normal map, or `NULL` to preserve the previous texture.
 */
void R3D_SetMapNormal(R3D_Model* model, int surfaceIndex, const Texture2D* texture);

/**
 * @brief Sets the ambient occlusion (AO) map for a specific surface in the model.
 * 
 * This function assigns an ambient occlusion texture to a specific surface of the model. The AO map affects how much light 
 * is occluded or blocked in crevices and corners of the surface. If `NULL` is provided for the texture, the previous texture 
 * or the default one will be preserved. The `lightAffect` parameter controls how much the ambient occlusion influences the 
 * final lighting.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param surfaceIndex The index of the surface to which the ambient occlusion map should be applied.
 * @param texture A pointer to the `Texture2D` object to be used as the ambient occlusion map, or `NULL` to preserve the previous texture.
 * @param lightAffect A float value controlling how much the ambient occlusion affects the surface lighting.
 */
void R3D_SetMapAO(R3D_Model* model, int surfaceIndex, const Texture2D* texture, float lightAffect);

/**
 * @brief Loads animations for a model from a file.
 * 
 * This function loads animations from a file and applies them to the model. The file typically contains animation data like
 * keyframes and transforms.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param fileName The name of the file containing the animation data.
 */
void R3D_LoadModelAnimations(R3D_Model* model, const char* fileName);

/**
 * @brief Gets the number of animations available in a model.
 * 
 * This function returns the total number of animations available for a given model.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * 
 * @return The number of animations available in the model.
 */
int R3D_GetModelAnimationCount(const R3D_Model* model);

/**
 * @brief Loads the names of the animations available in a model.
 * 
 * This function retrieves the names of all the animations in the model. The names are returned as an array of strings, and 
 * the number of names is returned through the `count` parameter.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param count A pointer to an integer that will hold the number of animation names.
 * 
 * @return An array of strings representing the names of the animations in the model.
 */
char** R3D_LoadModelAnimationNames(const R3D_Model* model, int* count);

/**
 * @brief Frees the memory allocated for animation names.
 * 
 * This function frees the memory that was allocated by `R3D_LoadModelAnimationNames` for the array of animation names.
 * 
 * @param names An array of strings representing the animation names.
 * @param count The number of animation names.
 */
void R3D_UnloadAnimationNames(char** names, int count);

/**
 * @brief Updates the animation of a model to a specific frame.
 * 
 * This function updates the model's animation to the specified frame of the given animation name.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param name The name of the animation to update.
 * @param frame The frame index to which the animation should be updated.
 */
void R3D_UpdateModelAnimation(R3D_Model* model, const char* name, int frame);

/**
 * @brief Updates the axis-aligned bounding box (AABB) of a model.
 * 
 * This function recalculates the AABB of the model, optionally adding extra margin around the bounding box for safety or 
 * other reasons. The AABB represents the box that fully contains the model in 3D space.
 * 
 * @param model A pointer to the `R3D_Model` object.
 * @param extraMargin A float value representing the additional margin to add around the bounding box.
 */
void R3D_UpdateModelAABB(R3D_Model* model, float extraMargin);

/**
 * @brief Generates tangents for all the meshes in the model surfaces.
 * 
 * This function calculates and generates tangent vectors for all meshes within the model. Tangents are essential for 
 * accurate normal mapping and texturing. This function computes the tangents based on the model's surfaces and their textures.
 * 
 * @param model A pointer to the `R3D_Model` object for which the tangents should be generated.
 */
void R3D_GenTangents(R3D_Model* model);


/* Transform Functions */

/**
 * @brief Creates a transform with identity matrix and an optional parent transform.
 * 
 * This function creates a new transform initialized to the identity matrix. Optionally, a parent transform can be provided 
 * to build a hierarchical transformation structure. The identity matrix represents no transformation (no rotation, scale, or translation).
 * 
 * @param parent A pointer to the parent `R3D_Transform` object, or `NULL` if no parent is specified.
 * 
 * @return A new `R3D_Transform` initialized to the identity matrix.
 */
R3D_Transform R3D_CreateTransformIdentity(const R3D_Transform* parent);

/**
 * @brief Creates a transform from a given transformation matrix.
 * 
 * This function creates a transform from a specified transformation matrix. The matrix defines the translation, rotation, 
 * and scale transformations applied to an object in 3D space.
 * 
 * @param mat The transformation matrix to convert into a `R3D_Transform` object.
 * 
 * @return A new `R3D_Transform` initialized from the provided matrix.
 */
R3D_Transform R3D_TransformFromMatrix(Matrix mat);

/**
 * @brief Converts a transform to a local transformation matrix.
 * 
 * This function converts the provided transform to its local space matrix. Local space refers to the object's own coordinate 
 * system, with transformations relative to its parent or origin.
 * 
 * @param transform A pointer to the `R3D_Transform` object to convert.
 * 
 * @return A matrix representing the local transformation of the given `R3D_Transform`.
 */
Matrix R3D_TransformToLocal(const R3D_Transform* transform);

/**
 * @brief Converts a transform to a global transformation matrix.
 * 
 * This function converts the provided transform to a global space matrix. Global space refers to the world or scene coordinate 
 * system, where transformations are relative to the world origin.
 * 
 * @param transform A pointer to the `R3D_Transform` object to convert.
 * 
 * @return A matrix representing the global transformation of the given `R3D_Transform`.
 */
Matrix R3D_TransformToGlobal(const R3D_Transform* transform);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // R3D_H
