#version 330 core

//--- �Է� ������
in vec3 out_Color; //--- ���� ���̴����� ���� �÷� ��
in vec2 out_TexCoord; //--- ���� ���̴����� ���� �ؽ�ó ��ǥ
in vec3 FragPos; //--- ���� ������ ���� ��ġ
in vec3 Normal; //--- ���� ������ ���� ����

//--- ��� ������
out vec4 FragColor;

//--- ���� ����ü
struct Light {
    int type; // 0: Directional, 1: Point, 2: Spot
    
    vec3 position; // ������, ����Ʈ����Ʈ��
    vec3 direction; // ���౤, ����Ʈ����Ʈ��
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    // ���� ��� (������, ����Ʈ����Ʈ��)
    float constant;
    float linear;
    float quadratic;
    
    // ����Ʈ����Ʈ��
    float cutOff;
    float outerCutOff;
};

//--- ���� ����ü
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

//--- Uniform ������
uniform sampler2D textureSampler; //--- �ؽ�ó ���÷�
uniform bool useTexture; //--- �ؽ�ó ��� ����
uniform bool useLighting; //--- ���� ��� ����

// ���� ���� uniform
uniform int lightCount;
uniform Light lights[8]; // �ִ� 8�� ����
uniform Material material;
uniform vec3 viewPos; // ī�޶� ��ġ

//--- ���౤ ��� �Լ�
vec3 CalcDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 matDiffuse) {
    vec3 lightDir = normalize(-light.direction);
    
    // Ȯ�걤 ���
    float diff = max(dot(normal, lightDir), 0.0);
    
    // �ݻ籤 ��� (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // �� ���� ���
    vec3 ambient = light.ambient * material.ambient * matDiffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse * matDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    return ambient + diffuse + specular;
}

//--- ������ ��� �Լ�
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 matDiffuse) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Ȯ�걤 ���
    float diff = max(dot(normal, lightDir), 0.0);
    
    // �ݻ籤 ��� (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // ���� ���
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // �� ���� ���
    vec3 ambient = light.ambient * material.ambient * matDiffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse * matDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

//--- ����Ʈ����Ʈ ��� �Լ�
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 matDiffuse) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Ȯ�걤 ���
    float diff = max(dot(normal, lightDir), 0.0);
    
    // �ݻ籤 ��� (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // ���� ���
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // ����Ʈ����Ʈ ���� ���
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // �� ���� ���
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
    // �⺻ ���� ���� ����
    vec3 materialColor;
    if (useTexture) {
        vec4 texColor = texture(textureSampler, out_TexCoord);
        // ���� �׽�Ʈ
        if(texColor.a < 0.1)
            discard;
        materialColor = texColor.rgb;
    } else {
        materialColor = out_Color;
    }
    
    // ���� ���
    if (useLighting && lightCount > 0) {
        vec3 normal = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 result = vec3(0.0);
        
        // ��� ������ ���� ���
        for(int i = 0; i < lightCount && i < 8; i++) {
            if (lights[i].type == 0) {
                // ���౤ (�¾籤)
                result += CalcDirectionalLight(lights[i], normal, viewDir, materialColor);
            }
            else if (lights[i].type == 1) {
                // ������
                result += CalcPointLight(lights[i], normal, FragPos, viewDir, materialColor);
            }
            else if (lights[i].type == 2) {
                // ����Ʈ����Ʈ
                result += CalcSpotLight(lights[i], normal, FragPos, viewDir, materialColor);
            }
        }
        
        FragColor = vec4(result, 1.0);
    } else {
        // ���� ���� �⺻ ������
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