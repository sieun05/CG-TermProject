#version 330 core
//--- in_Position: attribute index 0
//--- in_Color: attribute index 1
//--- in_TexCoord: attribute index 2 (텍스처 좌표)

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1
layout (location = 2) in vec2 in_TexCoord; //--- 텍스처 좌표: attribute position 2

uniform mat4 uMVP; //--- MVP 행렬
uniform bool useTexture; //--- 텍스처 사용 여부

out vec3 out_Color; //--- 프래그먼트 셰이더로 전달
out vec2 out_TexCoord; //--- 텍스처 좌표 전달

void main()
{
    gl_Position = uMVP * vec4(in_Position, 1.0);
    out_Color = in_Color;
    out_TexCoord = in_TexCoord;
}