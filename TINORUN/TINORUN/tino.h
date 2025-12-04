#pragma once
#include "헤더.h"
#include "game_object.h"
#include "common.h"
#include <string>

class Tino : public GameObject {
public:
    Tino() = default;
    Tino(const std::string& objPath, const std::string& texturePath);
    ~Tino();
    Tino(const Tino& other) = delete;
    Tino& operator=(const Tino& other) = delete;

    // GameObject의 가상 함수들을 오버라이드
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;

    // 모델 로딩
    bool LoadOBJ(const std::string& objPath);
    bool LoadTexture(const std::string& texturePath);

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint textureID;
    BITMAPINFO* bmp;
    
    bool isLoaded;
    
    void SetupMesh();
};

// 유틸리티 함수들
// GLuint LoadTextureFromFile(const std::string& path);

