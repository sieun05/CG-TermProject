#pragma once
#include "헤더.h"
#include "game_object.h"
#include "common.h"
#include <string>

class Obstacle : public GameObject {
public:
	Obstacle() = default;
	Obstacle(const std::string& objPath, const std::string& texturePath);
	~Obstacle();
	Obstacle(const Obstacle& other) = delete;
	Obstacle& operator=(const Obstacle& other) = delete;

	// GameObject의 가상 함수들을 오버라이드
	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;

	// 모델 로딩
	bool LoadOBJ(const std::string& objPath);
	bool LoadTexture(const std::string& texturePath);

	// 장애물이 제거되어야 하는지 확인
	bool ShouldBeRemoved() const;

	// 이동 속도 설정
	void SetSpeed(float speed) { moveSpeed = speed; }

private:
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
	void SetupMesh();
};

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
};

// 장애물용 정육면체 초기화 함수
// void InitObstacleBuffer();

// 장애물용 전역 변수
extern GLuint VAO_obstacle;
extern GLuint VBO_obstacle[2];
extern GLuint EBO_obstacle;