// The output returns the actual distances between the center of the cube and the fragments
// This shader requires a depth buffer and a color attachment
// Used for shadow mapping for omni lights

#version 330 core

in vec3 fragPosition;
uniform vec3 viewPos;
out float distanceToFrag;

void main()
{
    distanceToFrag = length(fragPosition - viewPos);
}
