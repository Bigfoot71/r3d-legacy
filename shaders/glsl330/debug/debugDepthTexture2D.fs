#version 330 core

in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform float uNear;
uniform float uFar;

out vec4 FragColor;

void main()
{
    float depth = texture(uTexture, vTexCoord).r;
    depth = (2.0 * uNear * uFar) / (uFar + uNear - (depth * 2.0 - 1.0) * (uFar - uNear));
    FragColor = vec4(vec3(depth/uFar), 1.0);
}
