#version 330 core
// in_Position: attribute index 0
// in_Color: attribute index 1
// in_TexCoord: attribute index 2 (Texture coordinates)
// in_Normal: attribute index 3 (Normal vector)

layout (location = 0) in vec3 in_Position; // Position attribute
layout (location = 1) in vec3 in_Color; // Color attribute
layout (location = 2) in vec2 in_TexCoord; // Texture coordinate attribute
layout (location = 3) in vec3 in_Normal; // Normal vector attribute

uniform mat4 uMVP; // MVP matrix
uniform mat4 uModel; // Model matrix
uniform mat4 uView; // View matrix
uniform mat4 uProjection; // Projection matrix
uniform mat4 uLightSpaceMatrix; // Light's view-projection matrix for shadows
uniform bool useTexture; // Texture usage flag

out vec3 out_Color; // Pass to fragment shader
out vec2 out_TexCoord; // Pass texture coordinates
out vec3 out_Normal; // Normal in world space
out vec3 out_FragPos; // Fragment position in world space
out vec4 out_FragPosLightSpace; // Fragment position in light space for shadows

void main()
{
    gl_Position = uMVP * vec4(in_Position, 1.0);
    out_Color = in_Color;
    out_TexCoord = in_TexCoord;

    // Transform fragment position to world space
    out_FragPos = vec3(uModel * vec4(in_Position, 1.0));

    // Transform normal (using normal matrix for non-uniform scaling)
    out_Normal = mat3(transpose(inverse(uModel))) * in_Normal;

    // Calculate fragment position in light space for shadow mapping
    out_FragPosLightSpace = uLightSpaceMatrix * vec4(out_FragPos, 1.0);
}