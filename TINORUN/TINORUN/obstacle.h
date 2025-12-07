#pragma once
#include "Header.h"
#include "game_object.h"
#include "common.h"
#include "Light.h"  // 조명 시스템과 Material 클래스 사용을 위해 추가
#include <string>
#include <vector>
#include <memory>
#include <random>

// 기본 장애물 클래스 (부모 클래스)
class Obstacle : public GameObject {
public:
    enum class ObstacleType {
        CACTUS,
        TREE,
        MUSHROOM,
        BIRD
    };

protected:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint textureID;
    BITMAPINFO* bmp;

    float moveSpeed; // 이동 속도 (x축으로 이동)
    
    // 제거될 x 위치
    static const float REMOVAL_X_POSITION;
    // 생성될 x 위치
    static const float SPAWN_X_POSITION;
    
    bool isLoaded;

    // 경계 박스 렌더링용 VAO/VBO
    GLuint boundaryVAO = 0;
    GLuint boundaryVBO = 0;
    
    virtual void SetupMesh();
    void SetupBoundaryMesh(); // protected로 변경

public:
    Obstacle();
    Obstacle(const std::string& objPath, const std::string& texturePath);
    virtual ~Obstacle();

    // GameObject의 가상 함수들을 오버라이드
    virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    virtual void Update() override;
    virtual void OnCollision(GameObject* other) override;
    
    // 경계 박스를 와이어프레임으로 렌더링
    void DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc);
    
    // 모델 로드
    bool LoadOBJ(const std::string& objPath);
    bool LoadTexture(const std::string& texturePath);

    // 장애물이 제거되어야 하는지 확인
    bool ShouldBeRemoved() const;
    
    // 이동 속도 설정
    void SetSpeed(float speed) { moveSpeed = speed; }

    // 장애물 타입 반환 (각 파생 클래스에서 구현)
    virtual ObstacleType GetType() const = 0;
};

// 전역 속도 계산 함수 선언
float GetObstacleSpeed();

// 선인장 클래스
class Cactus : public Obstacle {
public:
    Cactus();
    Cactus(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::CACTUS; }
};

// 나무 클래스
class Tree : public Obstacle {
public:
    Tree();
    Tree(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::TREE; }
};

// 버섯 클래스
class Mushroom : public Obstacle {
public:
    Mushroom();
    Mushroom(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::MUSHROOM; }
};

// 새 클래스
class Bird : public Obstacle {
public:
    Bird();
    Bird(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::BIRD; }
};

// 장애물 생성기 클래스
class ObstacleSpawner : public GameObject {
private:
    float spawnTimer;
    float spawnInterval;
    
    // 랜덤 생성을 위한 변수들
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::uniform_real_distribution<float> random_spawnInterval;

public:
    ObstacleSpawner();
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    void SpawnObstacle();
    std::unique_ptr<Obstacle> CreateRandomObstacle();
};