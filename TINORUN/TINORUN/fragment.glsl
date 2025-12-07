#version 330 core

//--- 입력 변수들
in vec3 out_Color; //--- 정점 셰이더에서 받은 컬러 값
in vec2 out_TexCoord; //--- 정점 셰이더에서 받은 텍스처 좌표
in vec3 FragPos; //--- 월드 공간의 정점 위치
in vec3 Normal; //--- 월드 공간의 법선 벡터

//--- 조명 구조체
struct Light {
    int type; // 0: Directional, 1: Point, 2: Spot
    
    vec3 position; // 점광원, 스포트라이트용
    vec3 direction; // 평행광, 스포트라이트용
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    // 감쇠 계수 (점광원, 스포트라이트용)
    float constant;
    float linear;
    float quadratic;
    
    // 스포트라이트용
    float cutOff;
    float outerCutOff;
};

//--- 재질 구조체
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

//--- Uniform 변수들
uniform sampler2D textureSampler; //--- 텍스처 샘플러
uniform bool useTexture; //--- 텍스처 사용 여부
uniform bool useLighting; //--- 조명 사용 여부

// 조명 관련 uniform
uniform int lightCount;
uniform Light lights[8]; // 최대 8개 조명
uniform Material material;
uniform vec3 viewPos; // 카메라 위치

out vec4 FragColor; //--- 최종 컬러

//--- 평행광 계산 함수
vec3 CalcDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 matDiffuse) {
    vec3 lightDir = normalize(-light.direction);
    
    // 확산광 계산
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 반사광 계산 (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // 각 성분 계산
    vec3 ambient = light.ambient * material.ambient * matDiffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse * matDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    return ambient + diffuse + specular;
}

//--- 점광원 계산 함수
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 matDiffuse) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 확산광 계산
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 반사광 계산 (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // 감쇠 계산
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 각 성분 계산
    vec3 ambient = light.ambient * material.ambient * matDiffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse * matDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

//--- 스포트라이트 계산 함수
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 matDiffuse) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // 확산광 계산
    float diff = max(dot(normal, lightDir), 0.0);
    
    // 반사광 계산 (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // 감쇠 계산
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 스포트라이트 강도 계산
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 각 성분 계산
    vec3 ambient = light.ambient * material.ambient * matDiffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse * matDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ambient + diffuse + specular;
}

void main()
{
    // 기본 재질 색상 결정
    vec3 materialColor;
    if (useTexture) {
        vec4 texColor = texture(textureSampler, out_TexCoord);
        // 알파 테스트
        if(texColor.a < 0.1)
            discard;
        materialColor = texColor.rgb;
    } else {
        materialColor = out_Color;
    }
    
    // 조명 계산
    if (useLighting && lightCount > 0) {
        vec3 normal = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 result = vec3(0.0);
        
        // 모든 조명에 대해 계산
        for(int i = 0; i < lightCount && i < 8; i++) {
            if (lights[i].type == 0) {
                // 평행광 (태양광)
                result += CalcDirectionalLight(lights[i], normal, viewDir, materialColor);
            }
            else if (lights[i].type == 1) {
                // 점광원
                result += CalcPointLight(lights[i], normal, FragPos, viewDir, materialColor);
            }
            else if (lights[i].type == 2) {
                // 스포트라이트
                result += CalcSpotLight(lights[i], normal, FragPos, viewDir, materialColor);
            }
        }
        
        FragColor = vec4(result, 1.0);
    } else {
        // 조명 없이 기본 렌더링
        if (useTexture) {
            vec4 texColor = texture(textureSampler, out_TexCoord);
            if(texColor.a < 0.1)
                discard;
            FragColor = texColor;
        } else {
            FragColor = vec4(materialColor, 1.0);
        }
    }
}