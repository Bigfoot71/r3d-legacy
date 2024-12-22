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

#ifndef R3D_DETAIL_CUBE_HPP
#define R3D_DETAIL_CUBE_HPP

#include "./gl.hpp"
#include <rlgl.h>

namespace r3d {

class Cube
{
public:
    Cube();
    ~Cube();
    GLuint vao() const;
    GLuint vbo() const;
    GLuint ebo() const;
    void draw() const;

private:
    static inline GLuint sVAO = 0;
    static inline GLuint sVBO = 0;
    static inline GLuint sEBO = 0;
    static inline int sCounter = 0;

private:
    static constexpr float VERTICES[] = {
        // Positions              // Normals              // Texcoords
        -1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 1.0f,  // Front top-left
        -1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    0.0f, 0.0f,  // Front bottom-left
         1.0f,  1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 1.0f,  // Front top-right
         1.0f, -1.0f,  1.0f,     0.0f,  0.0f,  1.0f,    1.0f, 0.0f,  // Front bottom-right

        -1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 1.0f,  // Back top-left
        -1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    1.0f, 0.0f,  // Back bottom-left
         1.0f,  1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 1.0f,  // Back top-right
         1.0f, -1.0f, -1.0f,     0.0f,  0.0f, -1.0f,    0.0f, 0.0f,  // Back bottom-right
    };

    static constexpr unsigned short INDICES[] = {
        // Front face
        0, 1, 2, 2, 1, 3,
        // Back face
        4, 5, 6, 6, 5, 7,
        // Left face
        4, 5, 0, 0, 5, 1,
        // Right face
        2, 3, 6, 6, 3, 7,
        // Top face
        4, 0, 6, 6, 0, 2,
        // Bottom face
        1, 5, 3, 3, 5, 7
    };
};

/* Implementation */

inline Cube::Cube()
{
    if (sCounter++ > 0) {
        return;
    }

    sVAO = rlLoadVertexArray();
    rlEnableVertexArray(sVAO);

    sEBO = rlLoadVertexBufferElement(INDICES, sizeof(INDICES), false);
    sVBO = rlLoadVertexBuffer(VERTICES, sizeof(VERTICES), false);

    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, false, 8 * sizeof(float), 0);
    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, false, 8 * sizeof(float), 6 * sizeof(float));
    rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, false, 8 * sizeof(float), 3 * sizeof(float));

    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);

    rlDisableVertexArray();
}

inline Cube::~Cube()
{
    if (--sCounter == 0) {
        const GLuint buffers[] = {
            sVBO, sEBO
        };
        glDeleteBuffers(2, buffers);
        if (sVAO > 0) {
            glDeleteVertexArrays(1, &sVAO);
        }
    }
}

inline GLuint Cube::vao() const
{
    return sVAO;
}

inline GLuint Cube::vbo() const
{
    return sVBO;
}

inline GLuint Cube::ebo() const
{
    return sEBO;
}

inline void Cube::draw() const
{
    bool vao = rlEnableVertexArray(sVAO);

    if (!vao) {
        rlEnableVertexBuffer(sVBO);

        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION);

        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD, 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD);

        rlSetVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);

        rlEnableVertexBufferElement(sEBO);
    }

    rlDrawVertexArrayElements(0, 36, 0);

    if (vao) {
        rlDisableVertexArray();
    } else {
        rlDisableVertexBuffer();
        rlDisableVertexBufferElement();
    }
}

} // namespace r3d

#endif // R3D_DETAIL_CUBE_HPP
