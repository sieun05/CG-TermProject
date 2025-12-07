#version 330 core
// Shadow map vertex shader
// Only calculates depth from light's perspective

layout (location = 0) in vec3 in_Position; // Position attribute

uniform mat4 uLightSpaceMatrix; // Light's view-projection matrix
uniform mat4 uModel; // Model matrix

void main()
{
    gl_Position = uLightSpaceMatrix * uModel * vec4(in_Position, 1.0);
}
