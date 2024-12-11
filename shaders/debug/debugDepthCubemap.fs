#version 330
in vec2 vTexCoord;
uniform samplerCube uCubemap;
uniform float uMaxVal;
out vec4 FragColor;
void main()
{
    // Initialize depth value to zero
    float depth = 0.0;
    
    // Create local texture coordinates based on fragment's texture coordinates
    vec2 localTexCoord = vTexCoord;

    // Normalize texture coordinates for each quad
    localTexCoord.t = mod(localTexCoord.t * 3.0, 1.0);
    localTexCoord.s = mod(localTexCoord.s * 4.0, 1.0);
    
    // Determine which quad to draw based on the fragment's texture coordinates
    if (vTexCoord.s * 4.0 > 1.0 && vTexCoord.s * 4.0 < 2.0)
    {
        // Bottom (-y) quad
        if (vTexCoord.t * 3.0 < 1.0)
        {
            vec3 dir = vec3(localTexCoord.s * 2.0 - 1.0, -1.0, localTexCoord.t * 2.0 - 1.0);
            depth = texture(uCubemap, dir).r;
        }
        // Top (+y) quad
        else if (vTexCoord.t * 3.0 > 2.0)
        {
            vec3 dir = vec3(localTexCoord.s * 2.0 - 1.0, 1.0, -localTexCoord.t * 2.0 + 1.0);
            depth = texture(uCubemap, dir).r;
        }
        // Front (-z) quad
        else
        {
            vec3 dir = vec3(localTexCoord.s * 2.0 - 1.0, localTexCoord.t * 2.0 - 1.0, -1.0);
            depth = texture(uCubemap, dir).r;
        }
    }
    else if (vTexCoord.t * 3.0 > 1.0 && vTexCoord.t * 3.0 < 2.0)
    {
        // Left (-x) quad
        if (vTexCoord.s * 4.0 < 1.0)
        {
            vec3 dir = vec3(-1.0, localTexCoord.t * 2.0 - 1.0, -localTexCoord.s * 2.0 + 1.0);
            depth = texture(uCubemap, dir).r;
        }
        // Right (+x) quad
        else if (vTexCoord.s * 4.0 < 3.0)
        {
            vec3 dir = vec3(1.0, localTexCoord.t * 2.0 - 1.0, localTexCoord.s * 2.0 - 1.0);
            depth = texture(uCubemap, dir).r;
        }
        // Back (+z) quad
        else
        {
            vec3 dir = vec3(-localTexCoord.s * 2.0 + 1.0, localTexCoord.t * 2.0 - 1.0, 1.0);
            depth = texture(uCubemap, dir).r;
        }
    }
    else
    {
        // Discard the fragment to avoid rendering unnecessary borders
        discard;
    }
    
    // Output the final color based on the computed depth, normalized by uMaxVal
    FragColor = vec4(vec3(depth / uMaxVal), 1.0);
}
