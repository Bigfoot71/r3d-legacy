// This shader is used to convert equirectangular map into cubemap
// The main use is for skyboxes

#version 330 core

in vec3 vPosition;

uniform sampler2D uTexEquirectangular;

out vec4 FragColor;

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, -0.3183); // negative Y, to flip axis
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(vPosition));
    vec3 color = texture(uTexEquirectangular, uv).rgb;
    FragColor = vec4(color, 1.0);
}
