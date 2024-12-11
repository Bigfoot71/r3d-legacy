#version 330 core

const float WEIGHT[5] = float[] (
    0.227027,
    0.1945946,
    0.1216216,
    0.054054,
    0.016216
);

in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform bool uHorizontal;

out vec4 FragColor;

void main()
{             
    vec2 texOffset = 1.0 / textureSize(uTexture, 0); // gets size of single texel
    vec3 result = texture(uTexture, vTexCoord).rgb * WEIGHT[0]; // current fragment's contribution
    if (uHorizontal)
    {
        for(int i = 1; i < 5; i++)
        {
            result += texture(uTexture, vTexCoord + vec2(texOffset.x * i, 0.0)).rgb * WEIGHT[i];
            result += texture(uTexture, vTexCoord - vec2(texOffset.x * i, 0.0)).rgb * WEIGHT[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(uTexture, vTexCoord + vec2(0.0, texOffset.y * i)).rgb * WEIGHT[i];
            result += texture(uTexture, vTexCoord - vec2(0.0, texOffset.y * i)).rgb * WEIGHT[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
