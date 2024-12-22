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

#ifndef R3D_SKYBOX_HPP
#define R3D_SKYBOX_HPP

#include "../detail/gl_helper/gl_shader.hpp"
#include "../detail/rl_helper/rl_texture.hpp"

#include "../detail/shader_code.hpp"
#include "../detail/drawable_quad.hpp"
#include "../detail/drawable_cube.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <cassert>
#include <memory>
#include <string>

// TODO: Integrate an automatic system to generate the BRDF LUT texture only once and load it on each startup
// TODO: Do the same for irradiance and prefilter textures, but this time for each loaded environment map

// NOTE: The implementation is in 'src/skybox.cpp'

namespace r3d {

class Skybox
{
    friend class Environment;

public:
    Skybox(const std::string& skyboxTexturePath, CubemapLayout layout);
    Skybox(const std::string& hdrSkyboxTexturePath, int size);

    ~Skybox();

    void draw(const Quaternion& rotation) const;

    unsigned int getSkyboxCuebmapID() const {
        return mCubemap.id;
    }

    unsigned int getIrradianceCubemapID() const {
        return mIrradiance.id;
    }

    unsigned int getPrefilterCubemapID() const {
        return mPrefilter.id;
    }

    unsigned int getBrdfLUTTextureID() const {
        return sShared->texBrdfLUT.id;
    }

private:
    RLTexture mCubemap;             ///< The cubemap texture representing the skybox.
    RLTexture mIrradiance;          ///< The irradiance cubemap texture for diffuse lighting.
    RLTexture mPrefilter;

private:
    struct SharedData
    {
        std::array<Matrix, 6> matCubeViews;

        GLShader shaderSkybox;
        GLShader shaderPrefilter;
        GLShader shaderIrradianceConvolution;
        GLShader shaderEquirectangularToCubemap;

        RLTexture texBrdfLUT;

        GLuint FBO;     ///< Used for texture generation
        GLuint RBO;     ///< Render buffer for depth (FBO)

        Quad quad;
        Cube cube;

        SharedData();
        ~SharedData();

        void generateBrdfLUT();
    };

private:
    static inline std::unique_ptr<SharedData> sShared{};
    static inline int sInstanceCounter{};

private:
    void load(const std::string& skyboxTexturePath, CubemapLayout layout);
    void loadHDR(const std::string& hdrSkyboxTexturePath, int size);

    void generateIrradiance();
    void generatePrefilter();
};


/* Public r3d::Sykbox implementation */

inline Skybox::Skybox(const std::string& skyboxTexturePath, CubemapLayout layout)
{
    if (sInstanceCounter++ == 0) {
        sShared = std::make_unique<SharedData>();
    }
    load(skyboxTexturePath, layout);
}

inline Skybox::Skybox(const std::string& hdrSkyboxTexturePath, int size)
{
    if (sInstanceCounter++ == 0) {
        sShared = std::make_unique<SharedData>();
    }
    loadHDR(hdrSkyboxTexturePath.c_str(), size);
}

inline Skybox::~Skybox()
{
    if (--sInstanceCounter == 0) {
        sShared.reset();
    }
}

inline void Skybox::draw(const Quaternion& rotation) const
{
    const GLShader& shader = sShared->shaderSkybox;

    // Bind shader program
    shader.begin();

    rlDisableBackfaceCulling();
    rlDisableDepthMask();

    // Get current view/projection matrices
    Matrix matView = rlGetMatrixModelview();
    Matrix matProj = rlGetMatrixProjection();

    // Bind cubemap texture
    shader.bindTexture("uTexSkybox", GL_TEXTURE_CUBE_MAP, mCubemap.id);

    // Set skybox parameters
    shader.setValue("uRotation", rotation);

    // Try binding vertex array objects (VAO) or use VBOs if not possible
    if (!rlEnableVertexArray(sShared->cube.vao())) {
        rlEnableVertexBuffer(sShared->cube.vbo());
        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
        rlEnableVertexBufferElement(sShared->cube.ebo());
    }

    // Draw skybox (supporting stereo rendering)
    if (rlIsStereoRenderEnabled()) {
        for (int eye = 0; eye < 2; eye++) {
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            shader.setValue("uMatView", MatrixMultiply(matView, rlGetMatrixViewOffsetStereo(eye)));
            shader.setValue("uMatProj", rlGetMatrixProjectionStereo(eye));
            rlDrawVertexArrayElements(0, 36, 0);
        }
    } else {
        shader.setValue("uMatView", matView);
        shader.setValue("uMatProj", matProj);
        rlDrawVertexArrayElements(0, 36, 0);
    }

    // Unbind cubemap texture
    rlActiveTextureSlot(0);
    rlDisableTextureCubemap();

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    shader.end();

    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

/* Private r3d::Skybox implementation */

inline void Skybox::load(const std::string& skyboxTexturePath, CubemapLayout layout)
{
    // Load the cubemap texture from the image file
    Image img = LoadImage(skyboxTexturePath.c_str());
    mCubemap = LoadTextureCubemap(img, layout);
    UnloadImage(img);

    // Generate maps
    generateIrradiance();
    generatePrefilter();
}

inline void Skybox::loadHDR(const std::string& hdrSkyboxTexturePath, int sizeFace)
{
    // Generate the cubemap for the skybox
    {
        // Load the HDR panorama texture
        Texture2D panorama = LoadTexture(hdrSkyboxTexturePath.c_str());

        // Create a renderbuffer for depth attachment
        glBindRenderbuffer(GL_RENDERBUFFER, sShared->RBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, sizeFace, sizeFace);

        // Create a cubemap texture to hold the HDR data
        glGenTextures(1, &mCubemap.id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mCubemap.id);
        for (int i = 0; i < 6; i++) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F,
                sizeFace, sizeFace, 0, GL_RGBA, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Configure the framebuffer with the renderbuffer and cubemap texture
        glBindFramebuffer(GL_FRAMEBUFFER, sShared->FBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sShared->RBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, mCubemap.id, 0);

        // Enable the shader to convert the HDR equirectangular map to cubemap faces
        sShared->shaderEquirectangularToCubemap.begin();

            // Set the projection matrix for the shader
            Matrix matProj = MatrixPerspective(90.0 * DEG2RAD, 1.0, 0.1, 10.0);
            sShared->shaderEquirectangularToCubemap.setValue("uMatProj", matProj);

            // Set the viewport to match the framebuffer dimensions
            glViewport(0, 0, sizeFace, sizeFace);
            glDisable(GL_CULL_FACE);

            // Activate the panorama texture
            sShared->shaderEquirectangularToCubemap.bindTexture(
                "uTexEquirectangular", GL_TEXTURE_CUBE_MAP, panorama.id
            );

            for (int i = 0; i < 6; i++) {
                // Set the view matrix for the current cubemap face
                sShared->shaderEquirectangularToCubemap.setValue("uMatView", sShared->matCubeViews[i]);

                // Attach the current cubemap face to the framebuffer
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mCubemap.id, 0);

                // Clear the framebuffer and draw the cube
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                sShared->cube.draw();
            }

        // Disable the shader and textures
        sShared->shaderEquirectangularToCubemap.end();

        // Reset the viewport to default dimensions
        glViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
        glEnable(GL_CULL_FACE);

        // Set the cubemap properties
        mCubemap.width = sizeFace;
        mCubemap.height = sizeFace;
        mCubemap.mipmaps = 1;
        mCubemap.format = RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;

        // Unload the panorama texture as it's no longer needed
        UnloadTexture(panorama);
    }

    // Generate maps
    generateIrradiance();
    generatePrefilter();
}

inline void Skybox::generateIrradiance()
{
    int size = std::max(mCubemap.width / 16, 32);

    // Create a renderbuffer for depth attachment
    glBindRenderbuffer(GL_RENDERBUFFER, sShared->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);

    // Create a cubemap texture to hold the HDR data
    glGenTextures(1, &mIrradiance.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mIrradiance.id);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F,
            size, size, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create and configure the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, sShared->FBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sShared->RBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, mIrradiance.id, 0);

    // Enable the shader for converting HDR equirectangular environment map to cubemap faces
    sShared->shaderIrradianceConvolution.begin();

        // Set the projection matrix for the shader
        Matrix matProj = MatrixPerspective(90.0 * DEG2RAD, 1.0, 0.1, 10.0);
        sShared->shaderIrradianceConvolution.setValue("uMatProj", matProj);

        // Set the viewport to match the framebuffer dimensions
        glViewport(0, 0, size, size);
        glDisable(GL_CULL_FACE); // Disable backface culling for cubemap rendering

        // Activate the panorama texture
        sShared->shaderIrradianceConvolution.bindTexture(
            "uTexCubemap", GL_TEXTURE_CUBE_MAP, mCubemap.id
        );

        for (int i = 0; i < 6; i++) {
            sShared->shaderIrradianceConvolution.setValue("uMatView", sShared->matCubeViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mIrradiance.id, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            sShared->cube.draw();
        }

    // Disable the shader and framebuffer
    sShared->shaderIrradianceConvolution.end();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset the viewport to default dimensions
    glViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight()); // Set to your default window dimensions
    glEnable(GL_CULL_FACE);

    // Set the cubemap properties
    mIrradiance.width = size;
    mIrradiance.height = size;
    mIrradiance.mipmaps = 1;
    mIrradiance.format = RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
}

inline void Skybox::generatePrefilter()
{
    // Create a cubemap texture to hold the HDR data
    glGenTextures(1, &mPrefilter.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mPrefilter.id);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
            128, 128, 0, GL_RGB, GL_FLOAT, nullptr
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    sShared->shaderPrefilter.begin();

        Matrix matProj = MatrixPerspective(90.0 * DEG2RAD, 1.0, 0.1, 10.0);
        sShared->shaderPrefilter.setValue("uMatProj", matProj);

        sShared->shaderPrefilter.bindTexture("uTexCubemap", GL_TEXTURE_CUBE_MAP, mCubemap.id);

        glBindFramebuffer(GL_FRAMEBUFFER, sShared->FBO);

        constexpr int MAX_MIP_LEVELS = 5;
        for (int mip = 0; mip < MAX_MIP_LEVELS; mip++) {
            int mipWidth = static_cast<int>(128 * std::pow(0.5, mip));
            int mipHeight = static_cast<int>(128 * std::pow(0.5, mip));

            glBindRenderbuffer(GL_RENDERBUFFER, sShared->RBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);

            glViewport(0, 0, mipWidth, mipHeight);
            glDisable(GL_CULL_FACE); // Disable backface culling for cubemap rendering

            float roughness = (float)mip / (float)(MAX_MIP_LEVELS - 1);
            sShared->shaderPrefilter.setValue("uRoughness", roughness);

            for (int i = 0; i < 6; i++) {
                sShared->shaderPrefilter.setValue("uMatView", sShared->matCubeViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mPrefilter.id, mip);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                sShared->cube.draw();
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    sShared->shaderPrefilter.end();

    // Reset the viewport to default dimensions
    glViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight()); // Set to your default window dimensions
    glEnable(GL_CULL_FACE);

    // Set the cubemap properties
    mPrefilter.width = 128;
    mPrefilter.height = 128;
    mPrefilter.mipmaps = 5;
    mPrefilter.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16;
}

/* Private r3d::Skybox::SharedData implementation */

inline Skybox::SharedData::SharedData()
    : shaderEquirectangularToCubemap(VS_CODE_CUBEMAP, FS_CODE_CUBEMAP_FROM_EQUIRECTANGULAR)
    , shaderIrradianceConvolution(VS_CODE_CUBEMAP, FS_CODE_IRRADIANCE_CONVOLUTION)
    , shaderPrefilter(VS_CODE_CUBEMAP, FS_CODE_PREFILTER)
    , shaderSkybox(VS_CODE_SKYBOX, FS_CODE_SKYBOX)
    , FBO(rlLoadFramebuffer())
{
    /* Compute cube view matrices for each faces */

    matCubeViews = {
        MatrixLookAt(Vector3 {}, Vector3 {  1.0f,  0.0f,  0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
        MatrixLookAt(Vector3 {}, Vector3 { -1.0f,  0.0f,  0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
        MatrixLookAt(Vector3 {}, Vector3 {  0.0f,  1.0f,  0.0f }, Vector3 { 0.0f,  0.0f,  1.0f }),
        MatrixLookAt(Vector3 {}, Vector3 {  0.0f, -1.0f,  0.0f }, Vector3 { 0.0f,  0.0f, -1.0f }),
        MatrixLookAt(Vector3 {}, Vector3 {  0.0f,  0.0f,  1.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
        MatrixLookAt(Vector3 {}, Vector3 {  0.0f,  0.0f, -1.0f }, Vector3 { 0.0f, -1.0f,  0.0f })
    };

    /* Check FBO for texture generation */

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", FBO);
    } else {
        TraceLog(LOG_ERROR, "FBO: [ID %i] Framebuffer object creation failed", FBO);
    }

    /* Create Render Buffer Object */

    glGenRenderbuffers(1, &RBO);

    /* Load VAO/VBO/EBO */

    constexpr float CUBE_POSITIONS[] =
    {
        // Front face
        -0.5f, -0.5f,  0.5f,    // Vertex 0
        0.5f, -0.5f,  0.5f,    // Vertex 1
        0.5f,  0.5f,  0.5f,    // Vertex 2
        -0.5f,  0.5f,  0.5f,    // Vertex 3

        // Back face
        -0.5f, -0.5f, -0.5f,    // Vertex 4
        0.5f, -0.5f, -0.5f,    // Vertex 5
        0.5f,  0.5f, -0.5f,    // Vertex 6
        -0.5f,  0.5f, -0.5f     // Vertex 7
    };

    // Define the indices for drawing the cube faces
    constexpr unsigned short CUBE_INDICES[] =
    {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Back face
        5, 4, 7,
        7, 6, 5,

        // Left face
        4, 0, 3,
        3, 7, 4,

        // Top face
        3, 2, 6,
        6, 7, 3,

        // Bottom face
        4, 5, 1,
        1, 0, 4
    };

    /* Generate BRDF LUT texture (see first TODO) */

    generateBrdfLUT();
}

inline Skybox::SharedData::~SharedData()
{
    glDeleteRenderbuffers(1, &RBO);
    rlUnloadFramebuffer(FBO);
}

inline void Skybox::SharedData::generateBrdfLUT()
{
    GLShader shaderBRDF(VS_CODE_BRDF, FS_CODE_BRDF);

    // Generate a 2D LUT from the BRDF equations used.
    glGenTextures(1, &texBrdfLUT.id);
    glBindTexture(GL_TEXTURE_2D, texBrdfLUT.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texBrdfLUT.id, 0);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderBRDF.begin();
        quad.draw();
    shaderBRDF.end();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    texBrdfLUT.width = 512;
    texBrdfLUT.height = 512;
    texBrdfLUT.mipmaps = 1;
    texBrdfLUT.format = RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16;  // R16G16 doesnt exist... (not very serious)
}

} // namespace r3d

#endif // R3D_SKYBOX_HPP
