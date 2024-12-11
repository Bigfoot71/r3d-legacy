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

#ifndef R3D_DETAIL_GL_HPP
#define R3D_DETAIL_GL_HPP

#include <stdexcept>
#include <string>

#if defined(PLATFORM_DESKTOP) || defined(PLATFORM_DESKTOP_SDL)
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #include "glad_gles2.h"       // Required for: OpenGL functionality 
        #define glGenVertexArrays glGenVertexArraysOES
        #define glBindVertexArray glBindVertexArrayOES
        #define glDeleteVertexArrays glDeleteVertexArraysOES
        #define GLSL_VERSION            100
    #else
        #if defined(__APPLE__)
            #define GL_SILENCE_DEPRECATION // Silence Opengl API deprecation warnings 
            #include <OpenGL/gl3.h>     // OpenGL 3 library for OSX
            #include <OpenGL/gl3ext.h>  // OpenGL 3 extensions library for OSX
        #else
            #include "glad.h"       // Required for: OpenGL functionality 
        #endif
        #define GLSL_VERSION            330
    #endif
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

namespace r3d {

/**
 * @brief Checks for OpenGL errors and throws an exception if an error is detected.
 * 
 * This function retrieves the latest OpenGL error using `glGetError()` and checks for any errors that have occurred.
 * If an error is found, it throws a `std::runtime_error` with a message describing the error and the context in which it occurred.
 * The context string is passed as a parameter to help identify where the error happened in the application.
 * 
 * @param context A string describing the context where the error occurred, typically the function or operation.
 */
inline void glCheckError(const char* context) {
    GLenum errCode = glGetError();
    if (errCode != GL_NO_ERROR) {
        std::string errorMessage;
        switch (errCode) {
            case GL_INVALID_ENUM:
                errorMessage = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
                break;
            case GL_INVALID_VALUE:
                errorMessage = "GL_INVALID_VALUE: A numeric argument is out of range.";
                break;
            case GL_INVALID_OPERATION:
                errorMessage = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorMessage = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
                break;
            case GL_OUT_OF_MEMORY:
                errorMessage = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
                break;
            case GL_STACK_UNDERFLOW:
                errorMessage = "GL_STACK_UNDERFLOW: A stack operation has caused an underflow.";
                break;
            case GL_STACK_OVERFLOW:
                errorMessage = "GL_STACK_OVERFLOW: A stack operation has caused an overflow.";
                break;
            default:
                errorMessage = "Unknown OpenGL error.";
                break;
        }
        throw std::runtime_error(errorMessage + " [" + context + "]");
    }
}

} // namespace r3d

#endif // R3D_DETAIL_GL_HPP
