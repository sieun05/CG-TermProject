#pragma once
#include "Header.h"
#include "game_object.h"
#include "common.h"
#include "Light.h"  // ���� �ý��۰� Material Ŭ���� ����� ���� �߰�
#include <string>
#include <vector>
#include <memory>
#include <random>

// �⺻ ��ֹ� Ŭ���� (�θ� Ŭ����)
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

    float moveSpeed; // �̵� �ӵ� (x������ �̵�)
    
    // ���ŵ� x ��ġ
    static const float REMOVAL_X_POSITION;
    // ������ x ��ġ
    static const float SPAWN_X_POSITION;
    
    bool isLoaded;

    // ��� �ڽ� �������� VAO/VBO
    GLuint boundaryVAO = 0;
    GLuint boundaryVBO = 0;
    
    virtual void SetupMesh();
    void SetupBoundaryMesh(); // protected�� ����

    bool backupSpawnFlag;

public:
    Obstacle();
    Obstacle(const std::string& objPath, const std::string& texturePath);
    virtual ~Obstacle();

    // GameObject�� ���� �Լ����� �������̵�
    virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    virtual void Update() override;
    virtual void OnCollision(GameObject* other) override;
    
    // ��� �ڽ��� ���̾����������� ������
    void DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc);
    
    // �� �ε�
    bool LoadOBJ(const std::string& objPath);
    bool LoadTexture(const std::string& texturePath);

    // ��ֹ��� ���ŵǾ�� �ϴ��� Ȯ��
    bool ShouldBeRemoved() const;
    
    // �̵� �ӵ� ����
    void SetSpeed(float speed) { moveSpeed = speed; }

    // ����
    void SetbackSpawnFlag(bool flag);

    // ��ֹ� Ÿ�� ��ȯ (�� �Ļ� Ŭ�������� ����)
    virtual ObstacleType GetType() const = 0;
};

// ���� �ӵ� ��� �Լ� ����
float GetObstacleSpeed();

// ������ Ŭ����
class Cactus : public Obstacle {
public:
    Cactus();
    Cactus(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::CACTUS; }
};

// ���� Ŭ����
class Tree : public Obstacle {
public:
    Tree();
    Tree(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::TREE; }
};

// ���� Ŭ����
class Mushroom : public Obstacle {
public:
    Mushroom();
    Mushroom(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::MUSHROOM; }
};

// �� Ŭ����
class Bird : public Obstacle {
public:
    Bird();
    Bird(const std::string& objPath, const std::string& texturePath);
    
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    ObstacleType GetType() const override { return ObstacleType::BIRD; }
};

// ��ֹ� ������ Ŭ����
class ObstacleSpawner : public GameObject {
private:
    float spawnTimer;
    float spawnInterval;
	bool backupSpawnFlag;
    
    // ���� ������ ���� ������
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;
    std::uniform_real_distribution<float> random_spawnInterval;

public:
    ObstacleSpawner();

	void SetbackSpawnFlag(bool flag) { backupSpawnFlag = flag; }

    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    void SpawnObstacle();
    std::unique_ptr<Obstacle> CreateRandomObstacle();
};