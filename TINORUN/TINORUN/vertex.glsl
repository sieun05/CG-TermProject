#version 330 core
//--- in_Position: attribute index 0
//--- in_Color: attribute index 1
//--- in_TexCoord: attribute index 2 (텍스처 좌표)
//--- in_Normal: attribute index 3 (법선 벡터)

layout (location = 0) in vec3 in_Position; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 in_Color; //--- 컬러 변수: attribute position 1
layout (location = 2) in vec2 in_TexCoord; //--- 텍스처 좌표: attribute position 2
layout (location = 3) in vec3 in_Normal; //--- 법선 벡터: attribute position 3

uniform mat4 uMVP; //--- MVP 행렬
uniform mat4 uModel; //--- 모델 행렬 (조명 계산용)
uniform mat4 uView; //--- 뷰 행렬
uniform mat4 uProjection; //--- 투영 행렬
uniform bool useTexture; //--- 텍스처 사용 여부
uniform bool useLighting; //--- 조명 사용 여부

out vec3 out_Color; //--- 프래그먼트 셰이더로 출력
out vec2 out_TexCoord; //--- 텍스처 좌표 출력
out vec3 FragPos; //--- 월드 공간의 정점 위치
out vec3 Normal; //--- 월드 공간의 법선 벡터

void main()
{
    // 정점 위치 변환
    gl_Position = uMVP * vec4(in_Position, 1.0);
    
    // 조명 계산을 위한 월드 공간 위치
    FragPos = vec3(uModel * vec4(in_Position, 1.0));
    
    // 법선 벡터를 월드 공간으로 변환 (비균등 스케일링 고려)
    Normal = mat3(transpose(inverse(uModel))) * in_Normal;
    
    // 기본 출력
    out_Color = in_Color;
    out_TexCoord = in_TexCoord;
}