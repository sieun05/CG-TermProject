#pragma once
#include "헤더.h"
#include "game_object.h"

class TempObstacle : public GameObject {
public:
    TempObstacle();
    ~TempObstacle() = default;
    TempObstacle(const TempObstacle& other) = default;
    TempObstacle& operator=(const TempObstacle& other) = default;

    // GameObject의 가상 함수들을 오버라이드
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;

    // 장애물이 제거되어야 하는지 확인
    bool ShouldBeRemoved() const;

    // 이동 속도 설정
    void SetSpeed(float speed) { moveSpeed = speed; }

private:
    float moveSpeed;        // 이동 속도 (x축으로 이동)
    static const float REMOVAL_X_POSITION; // 제거될 x 위치
    static const float SPAWN_X_POSITION;   // 생성될 x 위치
};

// 장애물 관리자 클래스
class ObstacleManager {
public:
    static ObstacleManager& GetInstance();
    
    void Update(float deltaTime);
    void SpawnObstacle();
    
    void SetSpawnInterval(float interval) { spawnInterval = interval; }
    
private:
    ObstacleManager() : spawnTimer(0.0f), spawnInterval(5.0f) {}
    
    float spawnTimer;
    float spawnInterval;
};

// 장애물용 정육면체 초기화 함수
void InitObstacleBuffer();

// 장애물용 전역 변수
extern GLuint VAO_obstacle;
extern GLuint VBO_obstacle[2];
extern GLuint EBO_obstacle;

