#include "Light.h"
#include <cmath>
#include <algorithm>
#include <string>  // to_string 함수 사용을 위해 추가

// 필요한 extern 선언들 (shader_func.h 대신)
extern GLuint shaderProgramID;

// 전역 조명 관리자 정의
LightManager g_lightManager;

// Light 구조체 구현
Light::Light() 
    : type(LightType::DIRECTIONAL),
      position(0.0f, 0.0f, 0.0f),
      direction(0.0f, -1.0f, 0.0f),
      ambient(0.1f, 0.1f, 0.1f),
      diffuse(0.8f, 0.8f, 0.8f),
      specular(1.0f, 1.0f, 1.0f),
      constant(1.0f),
      linear(0.09f),
      quadratic(0.032f),
      cutOff(12.5f),
      outerCutOff(17.5f)
{
}

Light Light::CreateDirectionalLight(
    const glm::vec3& direction,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular)
{
    Light light;
    light.type = LightType::DIRECTIONAL;
    light.direction = glm::normalize(direction);
    light.ambient = ambient;
    light.diffuse = diffuse;
    light.specular = specular;
    return light;
}

Light Light::CreatePointLight(
    const glm::vec3& position,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float constant,
    float linear,
    float quadratic)
{
    Light light;
    light.type = LightType::POINT;
    light.position = position;
    light.ambient = ambient;
    light.diffuse = diffuse;
    light.specular = specular;
    light.constant = constant;
    light.linear = linear;
    light.quadratic = quadratic;
    return light;
}

Light Light::CreateSpotLight(
    const glm::vec3& position,
    const glm::vec3& direction,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float cutOff,
    float outerCutOff)
{
    Light light;
    light.type = LightType::SPOT;
    light.position = position;
    light.direction = glm::normalize(direction);
    light.ambient = ambient;
    light.diffuse = diffuse;
    light.specular = specular;
    light.cutOff = cutOff;
    light.outerCutOff = outerCutOff;
    return light;
}

// Material 구조체 구현
Material::Material() 
    : ambient(0.2f, 0.2f, 0.2f),
      diffuse(0.8f, 0.8f, 0.8f),
      specular(0.0f, 0.0f, 0.0f),
      shininess(32.0f)
{
}

Material::Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin)
    : ambient(amb), diffuse(diff), specular(spec), shininess(shin)
{
}

Material Material::Gold() {
    return Material(
        glm::vec3(0.24725f, 0.1995f, 0.0745f),
        glm::vec3(0.75164f, 0.60648f, 0.22648f),
        glm::vec3(0.628281f, 0.555802f, 0.366065f),
        51.2f
    );
}

Material Material::Silver() {
    return Material(
        glm::vec3(0.19225f, 0.19225f, 0.19225f),
        glm::vec3(0.50754f, 0.50754f, 0.50754f),
        glm::vec3(0.508273f, 0.508273f, 0.508273f),
        51.2f
    );
}

Material Material::Bronze() {
    return Material(
        glm::vec3(0.2125f, 0.1275f, 0.054f),
        glm::vec3(0.714f, 0.4284f, 0.18144f),
        glm::vec3(0.393548f, 0.271906f, 0.166721f),
        25.6f
    );
}

Material Material::Plastic() {
    return Material(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.55f, 0.55f, 0.55f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        32.0f
    );
}

Material Material::Wood() {
    return Material(
        glm::vec3(0.19125f, 0.0735f, 0.0225f),
        glm::vec3(0.7038f, 0.27048f, 0.0828f),
        glm::vec3(0.256777f, 0.137622f, 0.086014f),
        12.8f
    );
}

Material Material::Grass() {
    return Material(
        glm::vec3(0.0f, 0.1f, 0.0f),
        glm::vec3(0.1f, 0.6f, 0.1f),
        glm::vec3(0.0f, 0.2f, 0.0f),
        8.0f
    );
}

Material Material::Stone() {
    return Material(
        glm::vec3(0.1f, 0.1f, 0.1f),
        glm::vec3(0.4f, 0.4f, 0.4f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        16.0f
    );
}

// LightManager 구현
LightManager::LightManager() : lightCount(0) {
    // uniform 위치들을 -1로 초기화
    uLightCount_loc = -1;
    uMaterialAmbient_loc = -1;
    uMaterialDiffuse_loc = -1;
    uMaterialSpecular_loc = -1;
    uMaterialShininess_loc = -1;
    uViewPos_loc = -1;
    uUseLighting_loc = -1;
    
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        uLightType_loc[i] = -1;
        uLightPosition_loc[i] = -1;
        uLightDirection_loc[i] = -1;
        uLightAmbient_loc[i] = -1;
        uLightDiffuse_loc[i] = -1;
        uLightSpecular_loc[i] = -1;
        uLightConstant_loc[i] = -1;
        uLightLinear_loc[i] = -1;
        uLightQuadratic_loc[i] = -1;
        uLightCutOff_loc[i] = -1;
        uLightOuterCutOff_loc[i] = -1;
    }
}

LightManager::~LightManager() {
}

void LightManager::InitializeUniforms(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    
    // 조명 개수
    uLightCount_loc = glGetUniformLocation(shaderProgram, "lightCount");
    
    // 각 조명의 uniform 위치들
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        std::string base = "lights[" + std::to_string(i) + "]";
        
        uLightType_loc[i] = glGetUniformLocation(shaderProgram, (base + ".type").c_str());
        uLightPosition_loc[i] = glGetUniformLocation(shaderProgram, (base + ".position").c_str());
        uLightDirection_loc[i] = glGetUniformLocation(shaderProgram, (base + ".direction").c_str());
        uLightAmbient_loc[i] = glGetUniformLocation(shaderProgram, (base + ".ambient").c_str());
        uLightDiffuse_loc[i] = glGetUniformLocation(shaderProgram, (base + ".diffuse").c_str());
        uLightSpecular_loc[i] = glGetUniformLocation(shaderProgram, (base + ".specular").c_str());
        uLightConstant_loc[i] = glGetUniformLocation(shaderProgram, (base + ".constant").c_str());
        uLightLinear_loc[i] = glGetUniformLocation(shaderProgram, (base + ".linear").c_str());
        uLightQuadratic_loc[i] = glGetUniformLocation(shaderProgram, (base + ".quadratic").c_str());
        uLightCutOff_loc[i] = glGetUniformLocation(shaderProgram, (base + ".cutOff").c_str());
        uLightOuterCutOff_loc[i] = glGetUniformLocation(shaderProgram, (base + ".outerCutOff").c_str());
    }
    
    // 재질 uniform 위치들
    uMaterialAmbient_loc = glGetUniformLocation(shaderProgram, "material.ambient");
    uMaterialDiffuse_loc = glGetUniformLocation(shaderProgram, "material.diffuse");
    uMaterialSpecular_loc = glGetUniformLocation(shaderProgram, "material.specular");
    uMaterialShininess_loc = glGetUniformLocation(shaderProgram, "material.shininess");
    
    // 뷰 위치
    uViewPos_loc = glGetUniformLocation(shaderProgram, "viewPos");
    
    // 조명 활성화
    uUseLighting_loc = glGetUniformLocation(shaderProgram, "useLighting");
    
    glUseProgram(0);
}

void LightManager::AddLight(const Light& light) {
    if (lightCount < MAX_LIGHTS) {
        lights[lightCount] = light;
        lightCount++;
    }
}

void LightManager::RemoveLight(int index) {
    if (index >= 0 && index < lightCount) {
        // 배열을 앞으로 이동
        for (int i = index; i < lightCount - 1; ++i) {
            lights[i] = lights[i + 1];
        }
        lightCount--;
    }
}

void LightManager::UpdateLight(int index, const Light& light) {
    if (index >= 0 && index < lightCount) {
        lights[index] = light;
    }
}

void LightManager::ClearLights() {
    lightCount = 0;
}

void LightManager::SendLightsToShader() {
    if (uLightCount_loc >= 0) {
        glUniform1i(uLightCount_loc, lightCount);
    }
    
    for (int i = 0; i < lightCount; ++i) {
        const Light& light = lights[i];
        
        if (uLightType_loc[i] >= 0) {
            glUniform1i(uLightType_loc[i], static_cast<int>(light.type));
        }
        if (uLightPosition_loc[i] >= 0) {
            glUniform3fv(uLightPosition_loc[i], 1, glm::value_ptr(light.position));
        }
        if (uLightDirection_loc[i] >= 0) {
            glUniform3fv(uLightDirection_loc[i], 1, glm::value_ptr(light.direction));
        }
        if (uLightAmbient_loc[i] >= 0) {
            glUniform3fv(uLightAmbient_loc[i], 1, glm::value_ptr(light.ambient));
        }
        if (uLightDiffuse_loc[i] >= 0) {
            glUniform3fv(uLightDiffuse_loc[i], 1, glm::value_ptr(light.diffuse));
        }
        if (uLightSpecular_loc[i] >= 0) {
            glUniform3fv(uLightSpecular_loc[i], 1, glm::value_ptr(light.specular));
        }
        if (uLightConstant_loc[i] >= 0) {
            glUniform1f(uLightConstant_loc[i], light.constant);
        }
        if (uLightLinear_loc[i] >= 0) {
            glUniform1f(uLightLinear_loc[i], light.linear);
        }
        if (uLightQuadratic_loc[i] >= 0) {
            glUniform1f(uLightQuadratic_loc[i], light.quadratic);
        }
        if (uLightCutOff_loc[i] >= 0) {
            glUniform1f(uLightCutOff_loc[i], glm::cos(glm::radians(light.cutOff)));
        }
        if (uLightOuterCutOff_loc[i] >= 0) {
            glUniform1f(uLightOuterCutOff_loc[i], glm::cos(glm::radians(light.outerCutOff)));
        }
    }
}

void LightManager::SendMaterialToShader(const Material& material) {
    if (uMaterialAmbient_loc >= 0) {
        glUniform3fv(uMaterialAmbient_loc, 1, glm::value_ptr(material.ambient));
    }
    if (uMaterialDiffuse_loc >= 0) {
        glUniform3fv(uMaterialDiffuse_loc, 1, glm::value_ptr(material.diffuse));
    }
    if (uMaterialSpecular_loc >= 0) {
        glUniform3fv(uMaterialSpecular_loc, 1, glm::value_ptr(material.specular));
    }
    if (uMaterialShininess_loc >= 0) {
        glUniform1f(uMaterialShininess_loc, material.shininess);
    }
}

void LightManager::SendViewPosition(const glm::vec3& viewPos) {
    if (uViewPos_loc >= 0) {
        glUniform3fv(uViewPos_loc, 1, glm::value_ptr(viewPos));
    }
}

void LightManager::EnableLighting(bool enable) {
    if (uUseLighting_loc >= 0) {
        glUniform1i(uUseLighting_loc, enable ? 1 : 0);
    }
}

void LightManager::SetupSunlight() {
    ClearLights();
    
    // y축 높은 곳에서 엄청나게 밝은 정오의 노란빛 태양
    Light sunlight = Light::CreateDirectionalLight(
        glm::vec3(0.0f, -1.0f, 0.0f),      // 정확히 위에서 아래로 향하는 태양
        glm::vec3(1.0f, 1.0f, 1.0f),       // 매우 강한 노란색 환경광
        glm::vec3(1.0f, 0.8f, 0.4f),       // 엄청나게 밝은 노란 직사광
        glm::vec3(2.2f, 2.0f, 1.6f)        // 매우 강한 노란색 반사광
    );
    
    AddLight(sunlight);
}

void LightManager::UpdateSunlight(float timeOfDay) {
    // 조명 색상 변경 완전 비활성화
    // 이 함수는 아무것도 하지 않음 - SetupSunlight()에서 설정한 색상 그대로 유지
    return;
}