// #version 330 core

/**
 * Additional Features
 *
 * VERTEX_COLOR
 * RECEIVE_SHADOW
 * MAP_NORMAL
 *
 */


#ifndef DIFFUSE_UNSHADED

// === General configuration ===

#define NUM_LIGHTS 8

// === Inputs ===

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

#ifdef VERTEX_COLOR
layout(location = 3) in vec4 aColor;
#endif

#ifdef MAP_NORMAL
layout(location = 4) in vec4 aTangent;
#endif

// === Uniforms ===

uniform mat4 uMatNormal;
uniform mat4 uMatModel;
uniform mat4 uMatMVP;

#ifdef RECEIVE_SHADOW
uniform mat4 uMatLightMVP[NUM_LIGHTS];
#endif

// === Outputs ===

out vec3 vPosition;
out vec2 vTexCoord;
out vec3 vNormal;

#ifdef MAP_NORMAL
out mat3 vTBN;
#endif

#ifdef VERTEX_COLOR
out vec4 vColor;
#endif

#ifdef RECEIVE_SHADOW
out vec4 vPosLightSpace[NUM_LIGHTS];
#endif

// === Main program ===

void main()
{
    vPosition = vec3(uMatModel * vec4(aPosition, 1.0));
    vNormal = normalize(vec3(uMatNormal * vec4(aNormal, 1.0)));
    vTexCoord = aTexCoord;

    #ifdef VERTEX_COLOR
        vColor = aColor;
    #endif

    #ifdef MAP_NORMAL
        // The TBN matrix is used to transform vectors from tangent space to world space
        // It is currently used to transform normals from a normal map to world space normals
        vec3 T = normalize(vec3(uMatModel * vec4(aTangent.xyz, 0.0)));
        vec3 B = cross(vNormal, T) * aTangent.w;
        vTBN = mat3(T, B, vNormal);
    #endif

    #ifdef RECEIVE_SHADOW
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            vPosLightSpace[i] = uMatLightMVP[i] * vec4(vPosition, 1.0);
        }
    #endif

    gl_Position = uMatMVP * vec4(aPosition, 1.0);
}


#else // DIFFUSE_UNSHADED

// === Inputs ===

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

#ifdef VERTEX_COLOR
layout(location = 3) in vec4 aColor;
#endif

// === Uniforms ===

uniform mat4 uMatMVP;

// === Outputs ===

out vec2 vTexCoord;

#ifdef VERTEX_COLOR
out vec4 vColor;
#endif

// === Main program ===

void main()
{
    vTexCoord = aTexCoord;

    #ifdef VERTEX_COLOR
        vColor = aColor;
    #endif

    gl_Position = uMatMVP * vec4(aPosition, 1.0);
}

#endif // DIFFUSE_UNSHADED
