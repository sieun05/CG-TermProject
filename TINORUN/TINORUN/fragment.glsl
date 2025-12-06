#version 330 core
//--- out_Color: 버텍스 셰이더에서 입력받는 컬러 값
//--- out_TexCoord: 버텍스 셰이더에서 입력받는 텍스처 좌표
//--- FragColor: 최종적 출력될 픽셀의 색상정보를 설정할 출력 변수

in vec3 out_Color; //--- 버텍스 셰이더에서 받은 컬러 변수
in vec2 out_TexCoord; //--- 버텍스 셰이더에서 받은 텍스처 좌표

uniform sampler2D textureSampler; //--- 텍스처 샘플러
uniform bool useTexture; //--- 텍스처 사용 여부

out vec4 FragColor; //--- 최종 컬러

void main()
{
    if (useTexture) {
        // 텍스처를 사용하는 경우
        vec4 texColor = texture(textureSampler, out_TexCoord);

        // 투명도 처리
        if(texColor.a < 0.1)
            discard;

        FragColor = texColor;
    } else {
        // 기본 컬러를 사용하는 경우
        FragColor = vec4(out_Color, 1.0);
    }
}