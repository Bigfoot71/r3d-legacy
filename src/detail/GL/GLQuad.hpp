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

#ifndef R3D_DETAIL_GL_QUAD_HPP
#define R3D_DETAIL_GL_QUAD_HPP

#include "../GL.hpp"

namespace r3d {

class GLQuad
{
public:
    GLQuad() {
        constexpr float VERTICES[] =
        {
            // Positions         Texcoords
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
            1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
            1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        };

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), &VERTICES, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)0); // Positions

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float))); // Texcoords
    }

    ~GLQuad() {
        glDeleteBuffers(1, &mVBO);
        glDeleteVertexArrays(1, &mVAO);
    }

    void draw() const {
        glBindVertexArray(mVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

private:
    GLuint mVAO = 0;
    GLuint mVBO = 0;
};

} // namespace r3d

#endif // R3D_DETAIL_GL_QUAD_HPP
