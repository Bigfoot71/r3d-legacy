// TODO: Replace runtime conditions with pre-processor conditions

#version 330 core

#define BLOOM_DISABLED 0
#define BLOOM_ADDITIVE 1
#define BLOOM_SOFT_LIGHT 2

#define FOG_DISABLED 0
#define FOG_LINEAR 1
#define FOG_EXP2 2
#define FOG_EXP 3

#define TONEMAP_LINEAR 0
#define TONEMAP_REINHARD 1
#define TONEMAP_FILMIC 2
#define TONEMAP_ACES 3

in vec2 vTexCoord;

uniform sampler2D uTexSceneHDR;
uniform sampler2D uTexSceneDepth;
uniform sampler2D uTexBloomBlurHDR;

uniform float uNear;
uniform float uFar;

uniform lowp int uBloomMode;
uniform float uBloomIntensity;

uniform lowp int uFogMode;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;

uniform lowp int uTonemapper;
uniform float uExposure;
uniform float uWhite;

uniform float uBrightness;
uniform float uContrast;
uniform float uSaturation;

out vec4 FragColor;


// === Helper functions === //

float LinearizeDepth(float depth, float near, float far)
{
    return (2.0 * near * far) / (far + near - (2.0 * depth - 1.0) * (far - near));;
}

float FogFactorLinear(float dist, float start, float end)
{
    return 1.0 - clamp((end - dist) / (end - start), 0.0, 1.0);
}

float FogFactorExp2(float dist, float density)
{
    const float LOG2 = -1.442695;
    float d = density * dist;
    return 1.0 - clamp(exp2(d * d * LOG2), 0.0, 1.0);
}

float FogFactorExp(float dist, float density)
{
    return 1.0 - clamp(exp(-density * dist), 0.0, 1.0);
}

//vec3 LinearToSRGB(vec3 color)
//{
//    //color = clamp(color, vec3(0.0), vec3(1.0));
//    //const vec3 a = vec3(0.055f);
//    //return mix((vec3(1.0f) + a) * pow(color.rgb, vec3(1.0f / 2.4f)) - a, 12.92f * color.rgb, lessThan(color.rgb, vec3(0.0031308f)));
//    // Approximation from http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
//    return max(vec3(1.055) * pow(color, vec3(0.416666667)) - vec3(0.055), vec3(0.0));
//}

// This expects 0-1 range input, outside that range it behaves poorly.
//vec3 SRGBToLinear(vec3 color)
//{
//    // Approximation from http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
//    return color * (color * (color * 0.305306011 + 0.682171111) + 0.012522878);
//}

// Based on Reinhard's extended formula, see equation 4 in https://doi.org/cjbgrt
vec3 TonemapReinhard(vec3 color, float pWhite)
{
    float whiteSquared = pWhite * pWhite;
    vec3 whiteSquaredColor = whiteSquared * color;
    // Equivalent to color * (1 + color / whiteSquared) / (1 + color)
    return (whiteSquaredColor + color * color) / (whiteSquaredColor + whiteSquared);
}

vec3 TonemapFilmic(vec3 color, float pWhite)
{
    // exposure bias: input scale (color *= bias, white *= bias) to make the brightness consistent with other tonemappers
    // also useful to scale the input to the range that the uTonemapper is designed for (some require very high input values)
    // has no effect on the curve's general shape or visual properties
    const float exposureBias = 2.0f;
    const float A = 0.22f * exposureBias * exposureBias; // bias baked into constants for performance
    const float B = 0.30f * exposureBias;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.01f;
    const float F = 0.30f;

    vec3 colorTonemapped = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float pWhiteTonemapped = ((pWhite * (A * pWhite + C * B) + D * E) / (pWhite * (A * pWhite + B) + D * F)) - E / F;

    return colorTonemapped / pWhiteTonemapped;
}

// Adapted from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// (MIT License).
vec3 TonemapACES(vec3 color, float pWhite)
{
    const float exposureBias = 1.8f;
    const float A = 0.0245786f;
    const float B = 0.000090537f;
    const float C = 0.983729f;
    const float D = 0.432951f;
    const float E = 0.238081f;

    // Exposure bias baked into transform to save shader instructions. Equivalent to `color *= exposureBias`
    const mat3 rgb_to_rrt = mat3(
        vec3(0.59719f * exposureBias, 0.35458f * exposureBias, 0.04823f * exposureBias),
        vec3(0.07600f * exposureBias, 0.90834f * exposureBias, 0.01566f * exposureBias),
        vec3(0.02840f * exposureBias, 0.13383f * exposureBias, 0.83777f * exposureBias)
    );

    const mat3 odt_to_rgb = mat3(
        vec3(1.60475f, -0.53108f, -0.07367f),
        vec3(-0.10208f, 1.10813f, -0.00605f),
        vec3(-0.00327f, -0.07276f, 1.07602f)
    );

    color *= rgb_to_rrt;
    vec3 colorTonemapped = (color * (color + A) - B) / (color * (C * color + D) + E);
    colorTonemapped *= odt_to_rgb;

    pWhite *= exposureBias;
    float pWhiteTonemapped = (pWhite * (pWhite + A) - B) / (pWhite * (C * pWhite + D) + E);

    return colorTonemapped / pWhiteTonemapped;
}


// === Post process functions === //

float FogFactor(float dist, int mode, float density, float start, float end)
{
    if (mode == FOG_LINEAR) return FogFactorLinear(dist, start, end);
    if (mode == FOG_EXP2) return FogFactorExp2(dist, density);
    if (mode == FOG_EXP) return FogFactorExp(dist, density);
    return 1.0; // FOG_DISABLED
}

vec3 Tonemapping(vec3 color, float pWhite) // inputs are LINEAR
{
    // Ensure color values passed to tonemappers are positive.
    // They can be negative in the case of negative lights, which leads to undesired behavior.
    if (uTonemapper == TONEMAP_REINHARD) return TonemapReinhard(max(vec3(0.0f), color), pWhite);
    if (uTonemapper == TONEMAP_FILMIC) return TonemapFilmic(max(vec3(0.0f), color), pWhite);
    if (uTonemapper == TONEMAP_ACES) return TonemapACES(max(vec3(0.0f), color), pWhite);
    return color; // TONEMAP_LINEAR
}


// === Main program === //

void main()
{
    // Echantilloner la texture de la scène
    vec3 result = texture(uTexSceneHDR, vTexCoord).rgb;

    // Application du bloom
    if (uBloomMode == BLOOM_SOFT_LIGHT)
    {
        vec3 bloom = texture(uTexBloomBlurHDR, vTexCoord).rgb;
        bloom *= uBloomIntensity;

        bloom = clamp(bloom.rgb, vec3(0.0f), vec3(1.0f));
	    result = max((result + bloom) - (result * bloom), vec3(0.0));
    }
    else if (uBloomMode == BLOOM_ADDITIVE)
    {
        vec3 bloom = texture(uTexBloomBlurHDR, vTexCoord).rgb;
        bloom *= uBloomIntensity;
        result += bloom;
    }

    if (uFogMode != FOG_DISABLED)
    {
        // Récupération de la profondeur et calcul de la distance
        float depth = texture(uTexSceneDepth, vTexCoord).r;
        depth = LinearizeDepth(depth, uNear, uFar);

        // Application du facteur de fog à la couleur resultante
        float fogFactor = FogFactor(depth, uFogMode, uFogDensity, uFogStart, uFogEnd);
        result = mix(result, uFogColor, fogFactor);
    }

    // Appliquer le tone mapping
    //result = SRGBToLinear(result);        // already linear
    result *= uExposure;
    result = Tonemapping(result, uWhite);
    //result = LinearToSRGB(result);        // see later

    // Apply gamma correction
    float gamma = 2.2;
    result = pow(result, vec3(1.0/gamma));

    // Ajustement de l'image
	result = mix(vec3(0.0), result, uBrightness);
	result = mix(vec3(0.5), result, uContrast);
	result = mix(vec3(dot(vec3(1.0), result) * 0.33333), result, uSaturation);

    // Finalisation de la couleur
    FragColor = vec4(result, 1.0);
}
