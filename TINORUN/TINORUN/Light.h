#pragma once
#include "Header.h"

// 조명 타입 열거형
enum class LightType {
    DIRECTIONAL,    // 평행광 (태양광)
    POINT,          // 점광원
    SPOT            // 스포트라이트
};

// 조명 구조체
struct Light {
    LightType type;
    
    // 조명 위치/방향
    glm::vec3 position;     // 점광원, 스포트라이트용
    glm::vec3 direction;    // 평행광, 스포트라이트용
    
    // 조명 색상과 강도
    glm::vec3 ambient;      // 환경광
    glm::vec3 diffuse;      // 확산광
    glm::vec3 specular;     // 반사광
    
    // 감쇠 계수 (점광원, 스포트라이트용)
    float constant;
    float linear;
    float quadratic;
    
    // 스포트라이트 전용
    float cutOff;           // 내부 원뿔각
    float outerCutOff;      // 외부 원뿔각
    
    // 기본 생성자
    Light();
    
    // 평행광 생성자 (태양광용)
    static Light CreateDirectionalLight(
        const glm::vec3& direction,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular
    );
    
    // 점광원 생성자
    static Light CreatePointLight(
        const glm::vec3& position,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular,
        float constant = 1.0f,
        float linear = 0.09f,
        float quadratic = 0.032f
    );
    
    // 스포트라이트 생성자
    static Light CreateSpotLight(
        const glm::vec3& position,
        const glm::vec3& direction,
        const glm::vec3& ambient,
        const glm::vec3& diffuse,
        const glm::vec3& specular,
        float cutOff = 12.5f,
        float outerCutOff = 17.5f
    );
};

// 재질 구조체
struct Material {
    glm::vec3 ambient;      // 환경광 반사계수
    glm::vec3 diffuse;      // 확산광 반사계수
    glm::vec3 specular;     // 반사광 반사계수
    float shininess;        // 광택도
    
    // 기본 생성자
    Material();
    
    // 매개변수 생성자
    Material(const glm::vec3& amb, const glm::vec3& diff, const glm::vec3& spec, float shin);
    
    // 미리 정의된 재질들
    static Material Gold();
    static Material Silver();
    static Material Bronze();
    static Material Plastic();
    static Material Wood();
    static Material Grass();
    static Material Stone();
};

// 조명 관리자 클래스
class LightManager {
private:
    static const int MAX_LIGHTS = 8;
    Light lights[MAX_LIGHTS];
    int lightCount;
    
    // 셰이더 uniform 위치들
    GLint uLightCount_loc;
    GLint uLightType_loc[MAX_LIGHTS];
    GLint uLightPosition_loc[MAX_LIGHTS];
    GLint uLightDirection_loc[MAX_LIGHTS];
    GLint uLightAmbient_loc[MAX_LIGHTS];
    GLint uLightDiffuse_loc[MAX_LIGHTS];
    GLint uLightSpecular_loc[MAX_LIGHTS];
    GLint uLightConstant_loc[MAX_LIGHTS];
    GLint uLightLinear_loc[MAX_LIGHTS];
    GLint uLightQuadratic_loc[MAX_LIGHTS];
    GLint uLightCutOff_loc[MAX_LIGHTS];
    GLint uLightOuterCutOff_loc[MAX_LIGHTS];
    
    // 재질 uniform 위치들
    GLint uMaterialAmbient_loc;
    GLint uMaterialDiffuse_loc;
    GLint uMaterialSpecular_loc;
    GLint uMaterialShininess_loc;
    
    // 뷰 위치 uniform
    GLint uViewPos_loc;
    
    // 조명 활성화 상태
    GLint uUseLighting_loc;
    
public:
    LightManager();
    ~LightManager();
    
    // 조명 관련 함수들
    void InitializeUniforms(GLuint shaderProgram);
    void AddLight(const Light& light);
    void RemoveLight(int index);
    void UpdateLight(int index, const Light& light);
    void ClearLights();
    
    // 셰이더에 조명 정보 전송
    void SendLightsToShader();
    void SendMaterialToShader(const Material& material);
    void SendViewPosition(const glm::vec3& viewPos);
    
    // 조명 활성화/비활성화
    void EnableLighting(bool enable);
    
    // 기본 태양광 설정
    void SetupSunlight();
    
    // 시간에 따른 태양광 업데이트 (하루 주기)
    void UpdateSunlight(float timeOfDay); // 0.0f ~ 1.0f (0=자정, 0.5=정오)
    
    // 접근자
    int GetLightCount() const { return lightCount; }
    const Light& GetLight(int index) const { return lights[index]; }
};

// 전역 조명 관리자
extern LightManager g_lightManager;