// #version 330 core

/**
 * Diffuse Modes
 *
 * DIFFUSE_UNSHADED
 * DIFFUSE_BURLEY
 * DIFFUSE_DISNEY
 * DIFFUSE_LAMBERT
 * DIFFUSE_PHONG
 * DIFFUSE_TOON
 *
 */

/**
 * Specular Modes
 *
 * SPECULAR_DISABLED
 * SPECULAR_SCHLICK_GGX
 * SPECULAR_DISNEY
 * SPECULAR_BLINN_PHONG
 * SPECULAR_TOON
 *
 */

/**
 * Additional Features
 *
 * VERTEX_COLOR
 * RECEIVE_SHADOW
 * MAP_EMISSION
 * MAP_NORMAL
 * MAP_AO
 * SKY_IBL
 *
 */


#ifndef DIFFUSE_UNSHADED

// === General configuration ===

#define PI 3.1415926535897932384626433832795028

#define NUM_LIGHTS  8

#define DIRLIGHT    0
#define SPOTLIGHT   1
#define OMNILIGHT   2

struct Light
{
    vec3 color;
    vec3 position;
    vec3 direction;
    float energy;
    float maxDistance;
    float attenuation;
    float innerCutOff;
    float outerCutOff;

#ifdef RECEIVE_SHADOW
    sampler2D shadowMap;
    samplerCube shadowCubemap;
    float shadowMapTxlSz;
    float shadowBias;
    bool shadow;
#endif

    lowp int type;
    bool enabled;
};

// === Inputs ===

in vec3 vPosition;
in vec2 vTexCoord;
in vec3 vNormal;

#ifdef VERTEX_COLOR
in vec4 vColor;
#endif

#ifdef MAP_NORMAL
in mat3 vTBN;
#endif

#ifdef RECEIVE_SHADOW
in vec4 vPosLightSpace[NUM_LIGHTS];
#endif

// === Outputs ===

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragBrightness;

// === Uniforms ===

uniform Light uLights[NUM_LIGHTS];

uniform float uBloomHdrThreshold;
uniform vec3 uColAmbient;
uniform vec3 uViewPos;

uniform sampler2D uTexAlbedo;
uniform vec4 uColAlbedo;

uniform sampler2D uTexMetalness;
uniform float uValMetalness;

uniform sampler2D uTexRoughness;
uniform float uValRoughness;

#ifdef MAP_EMISSION
uniform sampler2D uTexEmission;
uniform float uValEmissionEnergy;
uniform vec3 uColEmission;
#endif

#ifdef MAP_NORMAL
uniform sampler2D uTexNormal;
#endif

#ifdef MAP_AO
uniform sampler2D uTexAO;
uniform float uValAOLightAffect;
#endif

#ifdef SKY_IBL
uniform samplerCube uCubeIrradiance;
uniform samplerCube uCubePrefilter;
uniform sampler2D uTexBrdfLUT;
uniform bool uHasSkybox;
#endif

// === PBR functions ===

float DistributionGGX(float cosTheta, float alpha)
{
    // Standard GGX/Trowbridge-Reitz distribution - optimized form
    float a = cosTheta * alpha;
    float k = alpha / (1.0 - cosTheta * cosTheta + a * a);
    return k * k * (1.0 / PI);
}

float GTR2(float NdotH, float roughness)
{
    // Disney's GTR2 distribution (mathematically equivalent to GGX)
    // Matches the original Trowbridge-Reitz paper formulation
    // Used in Disney's principled BRDF
    float a2 = roughness * roughness;
    float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
    return a2 / (PI * t * t);
}

float GeometryGGXFast(float NdotL, float NdotV, float roughness)
{
    // Hammon's optimized approximation for GGX Smith geometry term
    // This version is an efficient approximation that:
    // 1. Avoids expensive square root calculations
    // 2. Combines both G1 terms into a single expression
    // 3. Provides very close results to the exact version at a much lower cost
    // SEE: https://www.gdcvault.com/play/1024478/PBR-Diffuse-Lighting-for-GGX
    return 0.5 / mix(2.0 * NdotL * NdotV, NdotL + NdotV, roughness);
}

float GeometryGGXExact(float NdotV, float roughness)
{
    // Disney's exact formulation of GGX Smith geometry term
    // This version is the physically accurate form that:
    // 1. Provides better physical accuracy, especially at grazing angles
    // 2. Uses separated Smith form (G = G1(NdotL) * G1(NdotV))
    // 3. Requires two evaluations (one for view, one for light)
    // 4. More computationally expensive due to square root
    float a = roughness * roughness;
    float b = NdotV * NdotV;
    return 1.0 / (NdotV + sqrt(a + b - a * b));
}

float SchlickFresnel(float u)
{
    float m = 1.0 - u;
    float m2 = m * m;
    return m2 * m2 * m; // pow(m,5)
}

vec3 SchlickFresnelRoughness(float cosTheta, vec3 F0, float roughness)
{
    // REVIEW: Can definitely be optimized
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 ComputeF0(float metallic, float specular, vec3 albedo)
{
    float dielectric = 0.16 * specular * specular;
    // use (albedo * metallic) as colored specular reflectance at 0 angle for metallic materials
    // SEE: https://google.github.io/filament/Filament.md.html
    return mix(vec3(dielectric), albedo, vec3(metallic));
}

// === Shadow functions ===

#ifdef RECEIVE_SHADOW

float ShadowOmni(int i, float cNdotL)
{
    vec3 lightToFrag = vPosition - uLights[i].position;

    float closestDepth = texture(uLights[i].shadowCubemap, lightToFrag).r;
    float currentDepth = length(lightToFrag);

    float bias = uLights[i].shadowBias * max(1.0 - cNdotL, 0.05);
    return (currentDepth - bias > closestDepth) ? 0.0 : 1.0;
}

float Shadow(int i, float cNdotL)
{
    vec4 p = vPosLightSpace[i];

    vec3 projCoords = p.xyz/p.w;
    projCoords = projCoords*0.5 + 0.5;

    float bias = max(uLights[i].shadowBias * (1.0 - cNdotL), 0.00002) + 0.00001;
    projCoords.z -= bias;

    float depth = projCoords.z;
    float shadow = 0.0;

    // NOTE: You can increase iterations to improve PCF quality
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(uLights[i].shadowMap, projCoords.xy + vec2(x, y) * uLights[i].shadowMapTxlSz).r;
            shadow += step(depth, pcfDepth);
        }
    }

    return shadow/9.0;
}

#endif // RECEIVE_SHADOW

// === Main program ===

void main()
{
    vec3 albedo = texture(uTexAlbedo, vTexCoord).rgb * uColAlbedo.rgb;

    #ifdef VERTEX_COLOR
        albedo *= vColor.rgb;
    #endif

    float roughness = uValRoughness * texture(uTexRoughness, vTexCoord).g;
    float metalness = uValMetalness * texture(uTexMetalness, vTexCoord).b;

    /* Compute F0 (reflectance at normal incidence) based on the metallic factor */

    vec3 F0 = ComputeF0(metalness, 0.5, albedo);

    /* Compute view direction and normal */

    vec3 V = normalize(uViewPos - vPosition);

    #ifdef MAP_NORMAL
        vec3 N = normalize(vTBN * (texture(uTexNormal, vTexCoord).rgb * 2.0 - 1.0));
    #else
        vec3 N = normalize(vNormal);
    #endif

    /* Compute the dot product of the normal and view direction */

    float NdotV = dot(N, V);
    float cNdotV = max(NdotV, 1e-4);  // Clamped to avoid division by zero

    /* Loop through all light sources accumulating diffuse and specular light */

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        if (uLights[i].enabled)
        {
            /* Compute light direction */

            vec3 L = vec3(0.0);
            if (uLights[i].type == DIRLIGHT) L = -uLights[i].direction;
            else L = normalize(uLights[i].position - vPosition);

            /* Compute the dot product of the normal and light direction */

            float NdotL = max(dot(N, L), 0.0);
            float cNdotL = min(NdotL, 1.0); // clamped NdotL

            /* Compute the halfway vector between the view and light directions */

            vec3 H = normalize(V + L);

            float LdotH = max(dot(L, H), 0.0);
            float cLdotH = min(dot(L, H), 1.0);

            float NdotH = max(dot(N, H), 0.0);
            float cNdotH = min(NdotH, 1.0);

            /* Compute light color energy */

            vec3 lightColE = uLights[i].color * uLights[i].energy;

            /* Compute diffuse lighting */

            vec3 diffLight = vec3(0.0);

            #if defined(DIFFUSE_BURLEY)
            {
                if (metalness < 1.0)
                {
                    float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
                    float FdV = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotV);
                    float FdL = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotL);

                    float diffBRDF = (1.0 / PI) * (FdV * FdL * cNdotL);
                    diffLight = diffBRDF * lightColE;
                }
            }
            #elif defined(DIFFUSE_DISNEY)
            {
                if (metalness < 1.0)
                {
                    float FD90 = 0.5 + 2.0 * roughness * cLdotH * cLdotH;  // FD90 factor with roughness influence
                    float FdV = 1.0 + (FD90 - 1.0) * SchlickFresnel(cNdotV);
                    float FdL = 1.0 + (FD90 - 1.0) * SchlickFresnel(cNdotL);

                    float energyBias = 0.5 * roughness;
                    float energyFactor = 1.0 - 0.5 * roughness;
                    float normalization = (1.0 / PI) * (energyBias + energyFactor * cLdotH * cLdotH);

                    // Diffuse BRDF with energy-conserving normalization
                    float diffBRDF = normalization * (FdV * FdL * cNdotL);
                    diffLight = diffBRDF * lightColE;
                }
            }
            #elif defined(DIFFUSE_LAMBERT)
            {
                if (metalness < 1.0)
                {
                    float diffBRDF = (1.0 / PI) * cNdotL;  // Diffuse BRDF constant * cosTheta
                    diffLight = diffBRDF * lightColE;
                }
            }
            #elif defined(DIFFUSE_PHONG)
            {
                diffLight = lightColE * NdotL;
                diffLight *= (1.0 - metalness);
            }
            #elif defined(DIFFUSE_TOON)
            {
                diffLight = lightColE * (0.5 * smoothstep(0.66, 0.67, NdotL) + 0.5);
                diffLight *= (1.0 - metalness);
            }
            #endif

            /* Compute specular lighting */

            vec3 specLight = vec3(0.0);

            #if defined(SPECULAR_SCHLICK_GGX)
            {
                // NOTE: When roughness is 0, specular light should not be entirely disabled.
                // TODO: Handle perfect mirror reflection when roughness is 0.

                if (roughness > 0.0)
                {
                    float alphaGGX = roughness * roughness;
                    float D = DistributionGGX(cNdotH, alphaGGX);
                    float G = GeometryGGXFast(cNdotL, cNdotV, alphaGGX);

                    float cLdotH5 = SchlickFresnel(cLdotH);
                    float F90 = clamp(50.0 * F0.g, 0.0, 1.0);
                    vec3 F = F0 + (F90 - F0) * cLdotH5;

                    vec3 specBRDF = cNdotL * D * F * G;
                    specLight = specBRDF * lightColE; // (specLight) * uLights[i].specular
                }
            }
            #elif defined(SPECULAR_DISNEY)
            {
                float alpha = max(0.001, roughness * roughness);

                float D = GTR2(NdotH, alpha);
                float Vis = GeometryGGXExact(NdotV, alpha) * GeometryGGXExact(NdotL, alpha);

                float F90 = clamp(50.0 * F0.g, 0.0, 1.0);
                vec3 F = F0 + (vec3(F90) - F0) * pow(1.0 - LdotH, 5.0);

                vec3 specBRDF = D * F * Vis;
                specLight = specBRDF * NdotL * lightColE; // (specLight) * uLights[i].specular
            }
            #elif defined(SPECULAR_BLINN_PHONG)
            {
                float invRoughness = 1.0 - roughness;
                float shininess = invRoughness * invRoughness * 512.0;
                vec3 metalSpec = mix(vec3(1.0), albedo.rgb, uValMetalness);
                specLight = lightColE * metalSpec * pow(NdotH, max(shininess, 8.0));
                specLight *= invRoughness;
            }
            #elif defined(SPECULAR_TOON)
            {
                float invRoughness = 1.0 - roughness;
                float shininess = invRoughness * invRoughness * 512.0;
                vec3 metalSpec = mix(vec3(1.0), albedo.rgb, uValMetalness);
                specLight = lightColE * metalSpec * step(0.5, pow(NdotH, max(shininess, 8.0)));
                specLight *= invRoughness;
            }
            #endif

            /* Apply shadow factor if the light casts shadows */

            float shadow = 1.0;

            #ifdef RECEIVE_SHADOW
                if (uLights[i].shadow)
                {
                    if (uLights[i].type != OMNILIGHT) shadow = Shadow(i, cNdotL);
                    else shadow = ShadowOmni(i, cNdotL);
                }
            #endif

            /* Apply attenuation based on the distance from the light */

            if (uLights[i].type != DIRLIGHT)
            {
                float dist = length(uLights[i].position - vPosition);
                float atten = 1.0 - clamp(dist / uLights[i].maxDistance, 0.0, 1.0);
                shadow *= atten * uLights[i].attenuation;
            }

            /* Apply spotlight effect if the light is a spotlight */

            if (uLights[i].type == SPOTLIGHT)
            {
                float theta = dot(L, -uLights[i].direction);
                float epsilon = (uLights[i].innerCutOff - uLights[i].outerCutOff);
                shadow *= smoothstep(0.0, 1.0, (theta - uLights[i].outerCutOff)/epsilon);
            }

            /* Accumulate the diffuse and specular lighting contributions */

            diffuse += diffLight * shadow;
            specular += specLight * shadow;
        }
    }

    /* Compute ambient - (IBL diffuse) */

    vec3 ambient = uColAmbient;

    #ifdef SKY_IBL
    {
        if (uHasSkybox)
        {
            vec3 kS = F0 + (1.0 - F0) * SchlickFresnel(cNdotV);
            vec3 kD = (1.0 - kS) * (1.0 - metalness);

            ambient = kD * texture(uCubeIrradiance, N).rgb;
        }
    }
    #endif

    #ifdef MAP_AO
    {
        float ao = texture(uTexAO, vTexCoord).r;
        ambient *= ao;

        float lightAffect = mix(1.0, ao, uValAOLightAffect);
        diffuse *= lightAffect;
        specular *= lightAffect;
    }
    #endif

    /* Skybox reflection - (IBL specular) */

    #ifdef SKY_IBL
    {
        if (uHasSkybox)
        {
            vec3 R = reflect(-V, N);
            const float MAX_REFLECTION_LOD = 4.0;
            vec3 prefilteredColor = textureLod(uCubePrefilter, R, roughness * MAX_REFLECTION_LOD).rgb;

            vec3 F = SchlickFresnelRoughness(cNdotV, F0, roughness);
            vec2 brdf = texture(uTexBrdfLUT, vec2(cNdotV, roughness)).rg;
            vec3 specularReflection = prefilteredColor * (F * brdf.x + brdf.y);

            specular += specularReflection;
        }
    }
    #endif

    /* Compute the final diffuse color, including ambient and diffuse lighting contributions */

    diffuse = albedo * (ambient + diffuse);

    /* Compute emission color; if an emissive map is used, sample it */

    vec3 emission = vec3(0.0);

    #ifdef MAP_EMISSION
    {
        vec3 texEmission = texture(uTexEmission, vTexCoord).rgb;
        emission = uColEmission * texEmission * uValEmissionEnergy;
    }
    #endif

    /* Compute the final fragment color by combining diffuse, specular, and emission contributions */

    FragColor = vec4(diffuse + specular + emission, 1.0);

    /* Handle bright colors for bloom / bloom */

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > uBloomHdrThreshold) FragBrightness = vec4(FragColor.rgb, 1.0);
    else FragBrightness = vec4(0.0, 0.0, 0.0, 1.0);
}


#else // DIFFUSE_UNSHADED

// === Inputs ===

in vec2 vTexCoord;

#ifdef VERTEX_COLOR
in vec4 vColor;
#endif

// === Outputs ===

out vec4 FragColor;

// === Uniforms ===

uniform sampler2D uTexAlbedo;
uniform vec4 uColAlbedo;

// === Main program ===

void main()
{
    vec4 color = texture(uTexAlbedo, vTexCoord);
    color *= uColAlbedo;

    #ifdef VERTEX_COLOR
        color *= vColor;
    #endif

    FragColor = color;
}

#endif // DIFFUSE_UNSHADED
