#version 330

layout (location = 0) in vec3 aPosition;

uniform mat4 uMatProj;
uniform mat4 uMatView;

out vec3 vPosition;

void main()
{
    vPosition = aPosition;
    mat4 rotView = mat4(mat3(uMatView));
    gl_Position = uMatProj * rotView * vec4(aPosition, 1.0);
}
