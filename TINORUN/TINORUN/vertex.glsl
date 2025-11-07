#version 330 core
//--- in_Position: attribute index 0
//--- in_Color: attribute index 1

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1

uniform mat4 uMVP; //--- MVP 행렬

out vec3 out_Color; //--- 프래그먼트 셰이더로 전달

void main()
{
    gl_Position = uMVP * vec4(in_Position, 1.0);
    out_Color = in_Color;
}