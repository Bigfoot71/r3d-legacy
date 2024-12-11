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

#ifndef R3D_DETAIL_SKYBOX_HPP
#define R3D_DETAIL_SKYBOX_HPP

#include "./GL.hpp"
#include "./helper/GL/GLShader.hpp"
#include "./helper/RL/RLTexture.hpp"

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

    void draw() const;

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

        GLuint VBO;     ///< VBO of cube positions
        GLuint EBO;     ///< EBO (elements) of the cube
        GLuint VAO;     ///< VAO -> VBO + EBO

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

} // namespace r3d

#endif // R3D_DETAIL_SKYBOX_HPP
