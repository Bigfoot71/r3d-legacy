/**
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

#ifndef R3D_DETAIL_MATERIAL_SHADER_HPP
#define R3D_DETAIL_MATERIAL_SHADER_HPP

#include "r3d.h"

#include "GL/GLFramebuffer.hpp"
#include "GL/GLShader.hpp"
#include "GL.hpp"

#include "../objects/skybox.hpp"
#include "../core/lighting.hpp"

#include <raylib.h>
#include <raymath.h>

#include <cstring>
#include <cstdlib>
#include <array>
#include <utility>

namespace r3d {

/**
 * @brief Maximum number of lights per surface, matching the value defined in the 'material' shader.
 * @note If you modify this value, ensure to update it in 'material.vs' and 'material.fs' shaders as well.
 */
static constexpr int SHADER_LIGHT_COUNT = 8;

/**
 * @brief Alias for an array of light pointers used in the shader.
 */
using ShaderLightArray = std::array<const Light*, SHADER_LIGHT_COUNT>;

/**
 * @class ShaderMaterial
 * @brief Class for managing the 'material.vs' / 'material.fs' shader.
 * 
 * This class is responsible for handling the shader associated with materials, including setting material properties, lighting, environment, and handling the shader program.
 */
class ShaderMaterial
{
public:
    /**
     * @brief Constructs a ShaderMaterial object with the specified material shader configuration.
     * @param config Configuration parameters for the material shader.
     */
    ShaderMaterial(R3D_MaterialShaderConfig config);

    // Destructor for ShaderMaterial
    ~ShaderMaterial();

    // Delete the copy constructor and assignment operator
    ShaderMaterial(const ShaderMaterial&) = delete;
    ShaderMaterial& operator=(const ShaderMaterial&) = delete;

    // Move constructor and assignment operator to allow moving of ShaderMaterial objects
    ShaderMaterial(ShaderMaterial&& other) noexcept;
    ShaderMaterial& operator=(ShaderMaterial&& other) noexcept;

    /**
     * @brief Begins the shader program by activating it for rendering.
     */
    void begin() const;

    /**
     * @brief Ends the shader program by deactivating it after rendering.
     */
    void end() const;

    /**
     * @brief Sets the environment parameters for the shader.
     * @param env The environment settings to be applied.
     * @param viewPos The camera/viewer's position in the world space.
     */
    void setEnvironment(const R3D_Environment& env, const Vector3& viewPos);

    /**
     * @brief Sets the material properties for the shader.
     * @param material The material settings to be applied.
     */
    void setMaterial(const R3D_Material& material);

    /**
     * @brief Sets the light sources for the shader.
     * @param lights The array of lights to be used by the shader.
     */
    void setLights(const ShaderLightArray& lights);

    /**
     * @brief Sets the model matrix for the shader.
     * @param matModel The model matrix to be applied.
     */
    void setMatModel(const Matrix& matModel);

    /**
     * @brief Sets the MVP (Model-View-Projection) matrix for the shader.
     * @param matMVP The MVP matrix to be applied.
     */
    void setMatMVP(const Matrix& matMVP);

private:
    /**
     * @brief Template class for managing shader uniform variables of type Type and GLType.
     * 
     * This class provides functionality to get and set uniform variables in the shader.
     */
    template <typename Type, GLenum GLType>
    class Uniform {
    public:
        /**
         * @brief Default constructor for the uniform class.
         */
        Uniform() = default;

        /**
         * @brief Constructor that sets the shader and uniform name.
         * @param shader The shader ID to associate the uniform with.
         * @param name The name of the uniform variable in the shader.
         */
        Uniform(GLuint shader, const char* name);

        /**
         * @brief Sets the value of the uniform variable.
         * @param value The value to set for the uniform.
         */
        void set(const Type& value);

        /**
         * @brief Gets the current value of the uniform variable.
         * @return The current value of the uniform.
         */
        const Type& get() const;

        /**
         * @brief Gets the current value of the uniform variable (non-const version).
         * @return The current value of the uniform.
         */
        Type& get();

    private:
        Type mValue{}; /**< The value of the uniform variable. */
        GLint mLoc = -1; /**< The location of the uniform variable in the shader. */
    };

    /**
     * @brief Template class for managing sampler units in the shader.
     * 
     * This class handles the binding and unbinding of textures to sampler units.
     */
    template <GLenum GLTraget>
    class Sampler {
    public:
        /**
         * @brief Default constructor for the sampler class.
         */
        Sampler() = default;

        /**
         * @brief Constructor that sets the shader, uniform name, and texture slot for the sampler.
         * @param shader The shader ID to associate the sampler with.
         * @param name The name of the sampler uniform in the shader.
         * @param slot The texture slot to bind the sampler to.
         */
        Sampler(GLuint shader, const char* name, GLuint slot);

        /**
         * @brief Binds a texture to the sampler unit.
         * @param texture The texture to bind.
         */
        void bind(GLuint texture) const;

        /**
         * @brief Unbinds the texture from the sampler unit.
         */
        void unbind() const;

    private:
        GLuint mSlot = 0; /**< The texture slot assigned to this sampler. */
    };

    /**
     * @struct Light
     * @brief Struct for storing light-related data used in the shader.
     * 
     * This structure holds all uniform variables and samplers related to lighting in the shader.
     */
    struct Light {
        Sampler<GL_TEXTURE_2D> shadowMap;           /**< Sampler for the shadow map texture. */
        Sampler<GL_TEXTURE_CUBE_MAP> shadowCubemap; /**< Sampler for the shadow cubemap texture. */
        Uniform<Matrix, GL_FLOAT_MAT4> matMVP;      /**< MVP matrix for the light's transformation. */
        Uniform<Color, GL_FLOAT_VEC3> color;        /**< Color of the light. */
        Uniform<Vector3, GL_FLOAT_VEC3> position;   /**< Position of the light (for point lights). */
        Uniform<Vector3, GL_FLOAT_VEC3> direction;  /**< Direction of the light (for directional lights). */
        Uniform<float, GL_FLOAT> energy;            /**< Energy/intensity of the light. */
        Uniform<float, GL_FLOAT> maxDistance;       /**< Maximum distance the light affects. */
        Uniform<float, GL_FLOAT> attenuation;       /**< Attenuation factor for light falloff. */
        Uniform<float, GL_FLOAT> innerCutOff;       /**< Inner cutoff for cone-shaped lights. */
        Uniform<float, GL_FLOAT> outerCutOff;       /**< Outer cutoff for cone-shaped lights. */
        Uniform<float, GL_FLOAT> shadowMapTxlSz;    /**< Texture size of the shadow map. */
        Uniform<float, GL_FLOAT> shadowBias;        /**< Bias for shadow calculations. */
        Uniform<int, GL_LOW_INT> type;              /**< Type of the light (e.g., point, directional). */
        Uniform<bool, GL_BOOL> shadow;              /**< Whether the light casts shadows. */
        Uniform<bool, GL_BOOL> enabled;             /**< Whether the light is enabled. */
    };

private:
    R3D_MaterialShaderConfig mConfig;               /**< The material shader configuration. */
    GLuint mShaderID;                               /**< The shader program ID. */

    std::array<Light, SHADER_LIGHT_COUNT> mLights;  /**< Array of light sources. */

    Uniform<Matrix, GL_FLOAT_MAT4> mMatNormal;      /**< Normal matrix for transforming normals. */
    Uniform<Matrix, GL_FLOAT_MAT4> mMatModel;       /**< Model matrix for object transformation. */
    Uniform<Matrix, GL_FLOAT_MAT4> mMatMVP;         /**< MVP matrix for transforming vertices. */

    Uniform<float, GL_FLOAT> mBloomHdrThreshold;    /**< Threshold for HDR bloom effect. */
    Uniform<Color, GL_FLOAT_VEC3> mColAmbient;      /**< Ambient color of the scene. */
    Uniform<Vector3, GL_FLOAT_VEC3> mViewPos;       /**< Position of the viewer (camera). */

    Sampler<GL_TEXTURE_2D> mTexAlbedo;              /**< Albedo texture sampler. */
    Uniform<Color, GL_FLOAT_VEC4> mColAlbedo;       /**< Albedo color. */

    Sampler<GL_TEXTURE_2D> mTexMetalness;           /**< Metalness texture sampler. */
    Uniform<float, GL_FLOAT> mValMetalness;         /**< Metalness value. */

    Sampler<GL_TEXTURE_2D> mTexRoughness;           /**< Roughness texture sampler. */
    Uniform<float, GL_FLOAT> mValRoughness;         /**< Roughness value. */

    Sampler<GL_TEXTURE_2D> mTexEmission;            /**< Emission texture sampler. */
    Uniform<float, GL_FLOAT> mValEmissionEnergy;    /**< Emission energy. */
    Uniform<Color, GL_FLOAT_VEC3> mColEmission;     /**< Emission color. */

    Sampler<GL_TEXTURE_2D> mTexNormal;              /**< Normal map texture sampler. */

    Sampler<GL_TEXTURE_2D> mTexAO;                  /**< Ambient occlusion texture sampler. */
    Uniform<float, GL_FLOAT> mValAOLightAffect;     /**< AO light affect value. */

    Sampler<GL_TEXTURE_CUBE_MAP> mCubeIrradiance;   /**< Cube map for irradiance. */
    Sampler<GL_TEXTURE_CUBE_MAP> mCubePrefilter;    /**< Cube map for prefiltered textures. */
    Sampler<GL_TEXTURE_2D> mTexBrdfLUT;             /**< BRDF LUT texture sampler. */
    Uniform<Vector4, GL_FLOAT_VEC4> mQuatSkybox;    /**< Skybox quaternion for environment mapping. */
    Uniform<bool, GL_BOOL> mHasSkybox;              /**< Whether the skybox is enabled. */
};


/* ShaderMaterial implementation */

inline ShaderMaterial::ShaderMaterial(R3D_MaterialShaderConfig config)
    : mConfig(config)
{
    std::string vsCode("#version 330 core\n");
    {
        if (config.flags & R3D_MATERIAL_FLAG_VERTEX_COLOR) {
            vsCode += "#define VERTEX_COLOR\n";
        }
        if (config.diffuse == R3D_DIFFUSE_UNSHADED) {
            vsCode += "#define DIFFUSE_UNSHADED\n";
        } else {
            if (config.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
                vsCode += "#define RECEIVE_SHADOW\n";
            }
            if (config.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
                vsCode += "#define MAP_NORMAL\n";
            }
        }

        vsCode += VS_CODE_MATERIAL;
    }

    std::string fsCode("#version 330 core\n");
    {
        switch (config.diffuse) {
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

        if (config.flags & R3D_MATERIAL_FLAG_VERTEX_COLOR) {
            fsCode += "#define VERTEX_COLOR\n";
        }

        if (config.diffuse != R3D_DIFFUSE_UNSHADED) {
            switch (config.specular) {
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
            if (config.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
                fsCode += "#define RECEIVE_SHADOW\n";
            }
            if (config.flags & R3D_MATERIAL_FLAG_MAP_EMISSION) {
                fsCode += "#define MAP_EMISSION\n";
            }
            if (config.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
                fsCode += "#define MAP_NORMAL\n";
            }
            if (config.flags & R3D_MATERIAL_FLAG_MAP_AO) {
                fsCode += "#define MAP_AO\n";
            }
            if (config.flags & R3D_MATERIAL_FLAG_SKY_IBL) {
                fsCode += "#define SKY_IBL\n";
            }
        }

        fsCode += FS_CODE_MATERIAL;
    }

    mShaderID = rlLoadShaderCode(vsCode.c_str(), fsCode.c_str());

    /* Get uniforms and init samplers */

    GLint textureSlot = 0;

    glUseProgram(mShaderID);

    mMatMVP = Uniform<Matrix, GL_FLOAT_MAT4>(mShaderID, "uMatMVP");

    mTexAlbedo = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexAlbedo", textureSlot++);
    mColAlbedo = Uniform<Color, GL_FLOAT_VEC4>(mShaderID, "uColAlbedo");

    if (config.diffuse == R3D_DIFFUSE_UNSHADED) {
        return;
    }

    mMatNormal = Uniform<Matrix, GL_FLOAT_MAT4>(mShaderID, "uMatNormal");
    mMatModel = Uniform<Matrix, GL_FLOAT_MAT4>(mShaderID, "uMatModel");

    mBloomHdrThreshold = Uniform<float, GL_FLOAT>(mShaderID, "uBloomHdrThreshold");
    mColAmbient = Uniform<Color, GL_FLOAT_VEC3>(mShaderID, "uColAmbient");
    mViewPos = Uniform<Vector3, GL_FLOAT_VEC3>(mShaderID, "uViewPos");

    mTexMetalness = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexMetalness", textureSlot++);
    mValMetalness = Uniform<float, GL_FLOAT>(mShaderID, "uValMetalness");

    mTexRoughness = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexRoughness", textureSlot++);
    mValRoughness = Uniform<float, GL_FLOAT>(mShaderID, "uValRoughness");

    if (config.flags & R3D_MATERIAL_FLAG_MAP_EMISSION) {
        mTexEmission = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexEmission", textureSlot++);
        mValEmissionEnergy = Uniform<float, GL_FLOAT>(mShaderID, "uValEmissionEnergy");
        mColEmission = Uniform<Color, GL_FLOAT_VEC3>(mShaderID, "uColEmission");
    }

    if (config.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
        mTexNormal = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexNormal", textureSlot++);
    }

    if (config.flags & R3D_MATERIAL_FLAG_MAP_AO) {
        mTexAO = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexAO", textureSlot++);
        mValAOLightAffect = Uniform<float, GL_FLOAT>(mShaderID, "uValAOLightAffect");
    }

    if (config.flags & R3D_MATERIAL_FLAG_SKY_IBL) {
        mCubeIrradiance = Sampler<GL_TEXTURE_CUBE_MAP>(mShaderID, "uCubeIrradiance", textureSlot++);
        mCubePrefilter = Sampler<GL_TEXTURE_CUBE_MAP>(mShaderID, "uCubePrefilter", textureSlot++);
        mTexBrdfLUT = Sampler<GL_TEXTURE_2D>(mShaderID, "uTexBrdfLUT", textureSlot++);
        mQuatSkybox = Uniform<Vector4, GL_FLOAT_VEC4>(mShaderID, "uQuatSkybox");
        mHasSkybox = Uniform<bool, GL_BOOL>(mShaderID, "uHasSkybox");
    }

    for (int i = 0; i < SHADER_LIGHT_COUNT; i++) {
        if (config.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
            mLights[i].shadowCubemap = Sampler<GL_TEXTURE_CUBE_MAP>(mShaderID, TextFormat("uLights[%i].shadowCubemap", i), textureSlot++);
            mLights[i].shadowMap = Sampler<GL_TEXTURE_2D>(mShaderID, TextFormat("uLights[%i].shadowMap", i), textureSlot++);
            mLights[i].matMVP = Uniform<Matrix, GL_FLOAT_MAT4>(mShaderID, TextFormat("uMatLightMVP[%i]", i));
            mLights[i].shadow = Uniform<bool, GL_BOOL>(mShaderID, TextFormat("uLights[%i].shadow", i));
        }
        mLights[i].color = Uniform<Color, GL_FLOAT_VEC3>(mShaderID, TextFormat("uLights[%i].color", i));
        mLights[i].position = Uniform<Vector3, GL_FLOAT_VEC3>(mShaderID, TextFormat("uLights[%i].position", i));
        mLights[i].direction = Uniform<Vector3, GL_FLOAT_VEC3>(mShaderID, TextFormat("uLights[%i].direction", i));
        mLights[i].energy = Uniform<float, GL_FLOAT>(mShaderID, TextFormat("uLights[%i].energy", i));
        mLights[i].maxDistance = Uniform<float, GL_FLOAT>(mShaderID, TextFormat("uLights[%i].maxDistance", i));
        mLights[i].attenuation = Uniform<float, GL_FLOAT>(mShaderID, TextFormat("uLights[%i].attenuation", i));
        mLights[i].innerCutOff = Uniform<float, GL_FLOAT>(mShaderID, TextFormat("uLights[%i].innerCutOff", i));
        mLights[i].outerCutOff = Uniform<float, GL_FLOAT>(mShaderID, TextFormat("uLights[%i].outerCutOff", i));
        mLights[i].shadowBias = Uniform<float, GL_FLOAT>(mShaderID, TextFormat("uLights[%i].shadowBias", i));
        mLights[i].type = Uniform<int, GL_LOW_INT>(mShaderID, TextFormat("uLights[%i].type", i));
        mLights[i].enabled = Uniform<bool, GL_BOOL>(mShaderID, TextFormat("uLights[%i].enabled", i));
    }
}

inline ShaderMaterial::~ShaderMaterial()
{
    if (mShaderID > 0) {
        glDeleteProgram(mShaderID);
    }
}

inline ShaderMaterial::ShaderMaterial(ShaderMaterial&& other) noexcept
    : mConfig(other.mConfig)
    , mShaderID(std::exchange(other.mShaderID, 0))
    , mLights(other.mLights)
    , mMatNormal(other.mMatNormal)
    , mMatModel(other.mMatModel)
    , mMatMVP(other.mMatMVP)
    , mBloomHdrThreshold(other.mBloomHdrThreshold)
    , mColAmbient(other.mColAmbient)
    , mViewPos(other.mViewPos)
    , mTexAlbedo(other.mTexAlbedo)
    , mColAlbedo(other.mColAlbedo)
    , mTexMetalness(other.mTexMetalness)
    , mValMetalness(other.mValMetalness)
    , mTexRoughness(other.mTexRoughness)
    , mValRoughness(other.mValRoughness)
    , mTexEmission(other.mTexEmission)
    , mValEmissionEnergy(other.mValEmissionEnergy)
    , mColEmission(other.mColEmission)
    , mTexNormal(other.mTexNormal)
    , mTexAO(other.mTexAO)
    , mValAOLightAffect(other.mValAOLightAffect)
    , mCubeIrradiance(other.mCubeIrradiance)
    , mCubePrefilter(other.mCubePrefilter)
    , mTexBrdfLUT(other.mTexBrdfLUT)
    , mQuatSkybox(other.mQuatSkybox)
    , mHasSkybox(other.mHasSkybox)
{ }

inline ShaderMaterial& ShaderMaterial::operator=(ShaderMaterial&& other) noexcept {
    if (this != &other) {
        mConfig = other.mConfig;
        mShaderID = std::exchange(other.mShaderID, 0);
        mLights = other.mLights;
        mMatNormal = other.mMatNormal;
        mMatModel = other.mMatModel;
        mMatMVP = other.mMatMVP;
        mBloomHdrThreshold = other.mBloomHdrThreshold;
        mColAmbient = other.mColAmbient;
        mViewPos = other.mViewPos;
        mTexAlbedo = other.mTexAlbedo;
        mColAlbedo = other.mColAlbedo;
        mTexMetalness = other.mTexMetalness;
        mValMetalness = other.mValMetalness;
        mTexRoughness = other.mTexRoughness;
        mValRoughness = other.mValRoughness;
        mTexEmission = other.mTexEmission;
        mValEmissionEnergy = other.mValEmissionEnergy;
        mColEmission = other.mColEmission;
        mTexNormal = other.mTexNormal;
        mTexAO = other.mTexAO;
        mValAOLightAffect = other.mValAOLightAffect;
        mCubeIrradiance = other.mCubeIrradiance;
        mCubePrefilter = other.mCubePrefilter;
        mTexBrdfLUT = other.mTexBrdfLUT;
        mQuatSkybox = other.mQuatSkybox;
        mHasSkybox = other.mHasSkybox;
    }
    return *this;
}

inline void ShaderMaterial::begin() const
{
    glUseProgram(mShaderID);
}

inline void ShaderMaterial::end() const
{
    glUseProgram(0);

    mTexAlbedo.unbind();

    if (mConfig.diffuse == R3D_DIFFUSE_UNSHADED) {
        return;
    }

    mTexMetalness.unbind();
    mTexRoughness.unbind();

    if (mConfig.flags & R3D_MATERIAL_FLAG_MAP_EMISSION) {
        mTexEmission.unbind();
    }

    if (mConfig.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
        mTexNormal.unbind();
    }

    if (mConfig.flags & R3D_MATERIAL_FLAG_MAP_AO) {
        mTexAO.unbind();
    }

    if (mConfig.flags & R3D_MATERIAL_FLAG_SKY_IBL) {
        mCubeIrradiance.unbind();
        mCubePrefilter.unbind();
        mTexBrdfLUT.unbind();
    }

    for (auto& light : mLights) {
        light.shadowCubemap.unbind();
        light.shadowMap.unbind();
    }
}

inline void ShaderMaterial::setEnvironment(const R3D_Environment& env, const Vector3& viewPos)
{
    bool skyAmbient = false;

    if (mConfig.flags & R3D_MATERIAL_FLAG_SKY_IBL) {
        mHasSkybox.set(env.world.skybox != nullptr);
        if (env.world.skybox != nullptr) {
            Skybox *sky = static_cast<Skybox*>(env.world.skybox->internal);
            mCubeIrradiance.bind(sky->getIrradianceCubemapID());
            mCubePrefilter.bind(sky->getPrefilterCubemapID());
            mTexBrdfLUT.bind(sky->getBrdfLUTTextureID());
            mQuatSkybox.set(QuaternionFromEuler(
                env.world.skybox->rotation.x * DEG2RAD,
                env.world.skybox->rotation.y * DEG2RAD,
                env.world.skybox->rotation.z * DEG2RAD
            ));
            skyAmbient = true;
        }
    }

    if (!skyAmbient) {
        mColAmbient.set(env.world.ambient);
    }

    if (env.bloom.mode != R3D_BLOOM_DISABLED) {
        mBloomHdrThreshold.set(env.bloom.hdrThreshold);
    }

    mViewPos.set(viewPos);
}

inline void ShaderMaterial::setMaterial(const R3D_Material& material)
{
    mTexAlbedo.bind(material.albedo.texture.id);
    mColAlbedo.set(material.albedo.color);

    if (mConfig.diffuse == R3D_DIFFUSE_UNSHADED) {
        return;
    }

    mTexMetalness.bind(material.metalness.texture.id);
    mValMetalness.set(material.metalness.factor);

    mTexRoughness.bind(material.roughness.texture.id);
    mValRoughness.set(material.roughness.factor);

    if (mConfig.flags & R3D_MATERIAL_FLAG_MAP_EMISSION) {
        mTexEmission.bind(material.emission.texture.id);
        mColEmission.set(material.emission.color);
        mValEmissionEnergy.set(material.emission.energy);
    }

    if (mConfig.flags & R3D_MATERIAL_FLAG_MAP_NORMAL) {
        mTexNormal.bind(material.normal.texture.id);
    }

    if (mConfig.flags & R3D_MATERIAL_FLAG_MAP_AO) {
        mTexAO.bind(material.ao.texture.id);
        mValAOLightAffect.set(material.ao.lightAffect);
    }
}

inline void ShaderMaterial::setLights(const ShaderLightArray& lights)
{
    if (mConfig.flags & R3D_DIFFUSE_UNSHADED) {
        return;
    }

    for (int i = 0; i < SHADER_LIGHT_COUNT; i++) {
        const r3d::Light *light = lights[i];
        Light& mLight = mLights[i];

        if (light == nullptr || !light->enabled) {
            mLight.enabled.set(false);
            continue;
        }

        mLight.enabled.set(true);
        mLight.color.set(light->color);
        mLight.energy.set(light->energy);
        mLight.type.set(static_cast<int>(light->type));

        if (mConfig.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW) {
            mLight.shadow.set(light->shadow);
            if (light->shadow) {
                mLight.shadowBias.set(light->shadowBias);
            }
        }

        switch (light->type) {
            case R3D_DIRLIGHT: {
                mLight.direction.set(light->direction);
                if (light->shadow && (mConfig.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW)) {
                    mLight.matMVP.set(light->vpMatrix());
                    mLight.shadowMapTxlSz.set(light->map->texelWidth());
                    mLight.shadowMap.bind(light->map->attachement(GLAttachement::DEPTH).id());
                }
            } break;
            case R3D_SPOTLIGHT: {
                mLight.position.set(light->position);
                mLight.direction.set(light->direction);
                mLight.maxDistance.set(light->maxDistance);
                mLight.attenuation.set(light->attenuation);
                mLight.innerCutOff.set(light->innerCutOff);
                mLight.outerCutOff.set(light->outerCutOff);
                if (light->shadow && (mConfig.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW)) {
                    mLight.matMVP.set(light->vpMatrix());
                    mLight.shadowMapTxlSz.set(light->map->texelWidth());
                    mLight.shadowMap.bind(light->map->attachement(GLAttachement::DEPTH).id());
                }
            } break;
            case R3D_OMNILIGHT: {
                mLight.position.set(light->position);
                mLight.maxDistance.set(light->maxDistance);
                mLight.attenuation.set(light->attenuation);
                if (light->shadow && (mConfig.flags & R3D_MATERIAL_FLAG_RECEIVE_SHADOW)) {
                    mLight.shadowCubemap.bind(light->map->attachement(GLAttachement::COLOR_0).id());
                }
            } break;
        }
    }
}

inline void ShaderMaterial::setMatModel(const Matrix& matModel)
{
    mMatNormal.set(MatrixTranspose(MatrixInvert(matModel)));
    mMatModel.set(matModel);
}

inline void ShaderMaterial::setMatMVP(const Matrix& matMVP)
{
    mMatMVP.set(matMVP);
}


/* ShaderMaterial::Uniform implementation */

template <typename Type, GLenum GLType>
ShaderMaterial::Uniform<Type, GLType>::Uniform(GLuint shader, const char* name)
    : mLoc(glGetUniformLocation(shader, name))
    , mValue()
{ }

template <typename Type, GLenum GLType>
void ShaderMaterial::Uniform<Type, GLType>::set(const Type& value)
{
    if constexpr (std::is_same_v<Type, bool>) {
        if (mValue != value) {
            glUniform1i(mLoc, static_cast<int>(value));
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, int>) {
        if (mValue != value) {
            glUniform1i(mLoc, value);
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, float>) {
        if (mValue != value) {
            glUniform1f(mLoc, value);
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, double>) {
        if (mValue != value) {
            glUniform1f(mLoc, static_cast<float>(value));
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, Vector2>) {
        if (mValue != value) {
            glUniform2f(mLoc, value.x, value.y);
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, Vector3>) {
        if (mValue != value) {
            glUniform3f(mLoc, value.x, value.y, value.z);
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, Vector4>) {
        if (mValue != value) {
            glUniform4f(mLoc, value.x, value.y, value.z, value.w);
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, Color>) {
        if (!ColorIsEqual(mValue, value)) {
            if constexpr (GLType == GL_FLOAT_VEC3) {
                glUniform3f(mLoc, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f);
            } else if constexpr (GLType == GL_FLOAT_VEC4) {
                glUniform4f(mLoc, value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f);
            }
            mValue = value;
        }
    } else if constexpr (std::is_same_v<Type, Matrix>) {
        rlSetUniformMatrix(mLoc, value);
        mValue = value;
    }
}

template <typename Type, GLenum GLType>
const Type& ShaderMaterial::Uniform<Type, GLType>::get() const
{
    return mValue;
}

template <typename Type, GLenum GLType>
Type& ShaderMaterial::Uniform<Type, GLType>::get()
{
    return mValue;
}


/* ShaderMaterial::Sampler implementation */

template <GLenum GLTarget>
ShaderMaterial::Sampler<GLTarget>::Sampler(GLuint shader, const char* name, GLuint slot)
    : mSlot(slot)
{
    glUniform1i(glGetUniformLocation(shader, name), slot);
}

template <GLenum GLTarget>
void ShaderMaterial::Sampler<GLTarget>::bind(GLuint texture) const
{
    glActiveTexture(GL_TEXTURE0 + mSlot);
    glBindTexture(GLTarget, texture);
}

template <GLenum GLTarget>
void ShaderMaterial::Sampler<GLTarget>::unbind() const
{
    glActiveTexture(GL_TEXTURE0 + mSlot);
    glBindTexture(GLTarget, 0);
}

} // namespace r3d

#endif // R3D_DETAIL_MATERIAL_SHADER_HPP
