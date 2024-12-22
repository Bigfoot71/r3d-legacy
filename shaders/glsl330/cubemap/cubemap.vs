#version 330 core

layout (location = 0) in vec3 aPosition;

uniform mat4 uMatProj;
uniform mat4 uMatView;

out vec3 vPosition;

void main()
{
    vPosition = aPosition;
    gl_Position = uMatProj * uMatView * vec4(vPosition, 1.0);
}
