#version 330

layout (location = 0) in vec3 aPosition;

uniform mat4 uMatProj;
uniform mat4 uMatView;
uniform vec4 uRotation;

out vec3 vPosition;

vec3 RotateWithQuat(vec3 v, vec4 q)
{
    vec3 t = 2.0 * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}

void main()
{
    vPosition = RotateWithQuat(aPosition, uRotation);

    mat4 rotView = mat4(mat3(uMatView));
    gl_Position = uMatProj * rotView * vec4(aPosition, 1.0);
}
