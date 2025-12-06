#pragma once
#include "Header.h"
#include "game_object.h"
#include "common.h"
#include <string>

// 기본 장애물 클래스 (부모 클래스)
class Obstacle : public GameObject {
public:
	Obstacle() = default;
	Obstacle(const std::string& objPath, const std::string& texturePath);
	virtual ~Obstacle();
	Obstacle(const Obstacle& other) = delete;
	Obstacle& operator=(const Obstacle& other) = delete;

	// GameObject의 가상 함수들을 오버라이드
	virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	virtual void Update() override;

	// 모델 로딩
	bool LoadOBJ(const std::string& objPath);
	bool LoadTexture(const std::string& texturePath);

	// 장애물이 제거되어야 하는지 확인
	bool ShouldBeRemoved() const;

	// 이동 속도 설정
	void SetSpeed(float speed) { moveSpeed = speed; }

	// 장애물 타입을 위한 열거형
	enum class ObstacleType {
		BASIC,
		CACTUS,
		TREE,
		MUSHROOM,
		BIRD
	};

	virtual ObstacleType GetType() const { return ObstacleType::BASIC; }

protected:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint textureID;
	BITMAPINFO* bmp;

	float moveSpeed;        // 이동 속도 (x축으로 이동)
	static const float REMOVAL_X_POSITION; // 제거될 x 위치
	static const float SPAWN_X_POSITION;   // 생성될 x 위치

	bool isLoaded;
	virtual void SetupMesh();
};







// obstacle class 상속받은 장애물들
class Cactus : public Obstacle {
public:
	Cactus();
	Cactus(const std::string& objPath, const std::string& texturePath);
	virtual ~Cactus() = default;

	virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	virtual void Update() override;
	virtual ObstacleType GetType() const override { return ObstacleType::CACTUS; }

private:
};

class Tree : public Obstacle {
public:
	Tree();
	Tree(const std::string& objPath, const std::string& texturePath);
	virtual ~Tree() = default;

	virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	virtual void Update() override;
	virtual ObstacleType GetType() const override { return ObstacleType::TREE; }

private:
};

class Mushroom : public Obstacle {
public:
	Mushroom();
	Mushroom(const std::string& objPath, const std::string& texturePath);
	virtual ~Mushroom() = default;

	virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	virtual void Update() override;
	virtual ObstacleType GetType() const override { return ObstacleType::MUSHROOM; }

private:
};

class Bird : public Obstacle {
public:
	Bird();
	Bird(const std::string& objPath, const std::string& texturePath);
	virtual ~Bird() = default;

	virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	virtual void Update() override;
	virtual ObstacleType GetType() const override { return ObstacleType::BIRD; }

private:
};




//스포너
class ObstacleSpawner : public GameObject {
public:
	ObstacleSpawner();
	~ObstacleSpawner() = default;

	// GameObject의 가상 함수들을 오버라이드
	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;

	void SetSpawnInterval(float interval) { spawnInterval = interval; }
	void SpawnObstacle();

private:
	float spawnTimer;
	float spawnInterval;
	
	// 랜덤하게 장애물 종류를 선택하기 위한 메서드
	std::unique_ptr<Obstacle> CreateRandomObstacle();
};

// 장애물용 정육면체 초기화 함수
// void InitObstacleBuffer();

// 장애물용 전역 변수
extern GLuint VAO_obstacle;
extern GLuint VBO_obstacle[2];
extern GLuint EBO_obstacle;