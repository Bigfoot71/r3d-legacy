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

#ifndef R3D_DETAIL_QUAD_HPP
#define R3D_DETAIL_QUAD_HPP

#include "./GL.hpp"
#include <rlgl.h>

namespace r3d {

class Quad
{
public:
    Quad();
    ~Quad();
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
        // Positions         Normals             Texcoords
       -1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        1.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    };

    static constexpr unsigned short INDICES[] = {
        0, 1, 2,  // First triangle (bottom-left, bottom-right, top-left)
        1, 3, 2   // Second triangle (bottom-right, top-right, top-left)
    };
};

/* Implementation */

inline Quad::Quad()
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

inline Quad::~Quad()
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

inline GLuint Quad::vao() const
{
    return sVAO;
}

inline GLuint Quad::vbo() const
{
    return sVBO;
}

inline GLuint Quad::ebo() const
{
    return sEBO;
}

inline void Quad::draw() const
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

    rlDrawVertexArrayElements(0, 6, 0);

    if (vao) {
        rlDisableVertexArray();
    } else {
        rlDisableVertexBuffer();
        rlDisableVertexBufferElement();
    }
}

} // namespace r3d

#endif // R3D_DETAIL_QUAD_HPP
