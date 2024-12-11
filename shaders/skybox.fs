#version 330

in vec3 vPosition;
uniform samplerCube uTexSkybox;
out vec4 FragColor;

void main()
{
    vec3 color = texture(uTexSkybox, vPosition).rgb;
    FragColor = vec4(color, 1.0);
}
