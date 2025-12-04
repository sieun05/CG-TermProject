#include "temp_obstacle.h"
#include "game_world.h"
#include "game_state.h"
#include <memory>

// 정적 상수 정의
const float TempObstacle::REMOVAL_X_POSITION = -20.0f;
const float TempObstacle::SPAWN_X_POSITION = 50.0f;

// 장애물용 전역 변수 정의
GLuint VAO_obstacle = 0;
GLuint VBO_obstacle[2] = {0, };
GLuint EBO_obstacle = 0;

void InitObstacleBuffer()
{
    const float size = 1.0f; // 정육면체 한 변의 절반 길이

    const float obstacle_vertices[] = {
        // 앞면 (z = +size)
        -size, -size,  size,  // 0: 왼쪽 아래
         size, -size,  size,  // 1: 오른쪽 아래
         size,  size,  size,  // 2: 오른쪽 위
        -size,  size,  size,  // 3: 왼쪽 위

        // 뒷면 (z = -size)
         size, -size, -size,  // 4: 오른쪽 아래
        -size, -size, -size,  // 5: 왼쪽 아래
        -size,  size, -size,  // 6: 왼쪽 위
         size,  size, -size,  // 7: 오른쪽 위
    };

    const float obstacle_colors[] = {
        // 앞면 - 빨간색
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // 뒷면 - 빨간색
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };

    const unsigned int obstacle_indices[] = {
        // 앞면
        0, 1, 2,   2, 3, 0,
        // 뒷면
        4, 5, 6,   6, 7, 4,
        // 왼쪽면
        5, 0, 3,   3, 6, 5,
        // 오른쪽면
        1, 4, 7,   7, 2, 1,
        // 아래면
        5, 4, 1,   1, 0, 5,
        // 위면
        3, 2, 7,   7, 6, 3
    };

    // VAO 생성 및 바인딩
    glGenVertexArrays(1, &VAO_obstacle);
    glBindVertexArray(VAO_obstacle);

    // VBO 생성
    glGenBuffers(2, VBO_obstacle);

    // 정점 위치 데이터
    glBindBuffer(GL_ARRAY_BUFFER, VBO_obstacle[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(obstacle_vertices), obstacle_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 정점 색상 데이터
    glBindBuffer(GL_ARRAY_BUFFER, VBO_obstacle[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(obstacle_colors), obstacle_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // EBO 설정
    glGenBuffers(1, &EBO_obstacle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_obstacle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(obstacle_indices), obstacle_indices, GL_STATIC_DRAW);

    // 바인딩 해제
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// TempObstacle 클래스 구현
TempObstacle::TempObstacle() 
    : moveSpeed(-5.0f) // x축 음의 방향으로 이동 (초당 5 단위)
{
    // 생성 위치 설정
    position = glm::vec3(SPAWN_X_POSITION, 2.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
    
    std::cout << "장애물 생성: 위치 (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void TempObstacle::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    // 텍스처 사용 모드 비활성화
    if (uUseTexture_loc >= 0) {
        glUniform1i(uUseTexture_loc, 0); // false
    }

    glBindVertexArray(VAO_obstacle);

    // 모델 매트릭스 계산
    glm::mat4 model = GetModelMatrix();
    glm::mat4 mvp = gProjection * gView * model;

    // 유니폼 설정
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 그리기
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void TempObstacle::Update()
{
    // x축으로 이동 (약 60FPS 기준 deltaTime = 0.016f 가정)
    const float deltaTime = 0.016f;
    position.x += moveSpeed * deltaTime;
    
    // 디버그 출력 (가끔씩만)
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) { // 1초마다 출력
        std::cout << "장애물 위치: x = " << position.x << std::endl;
    }
}

bool TempObstacle::ShouldBeRemoved() const
{
    return position.x <= REMOVAL_X_POSITION;
}

// ObstacleSpawner 클래스 구현
ObstacleSpawner::ObstacleSpawner()
    : spawnTimer(0.0f), spawnInterval(5.0f)
{
    // 스포너는 렌더링되지 않는 객체이므로 위치는 상관없음
    std::cout << "ObstacleSpawner 생성됨" << std::endl;
}

void ObstacleSpawner::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    // 스포너는 렌더링되지 않음
}

void ObstacleSpawner::Update()
{
    // PLAYING 상태일 때만 장애물 생성
    if (scene != GameState::PLAYING) {
        spawnTimer = 0.0f; // 다른 상태에서는 타이머 초기화
        return;
    }

    const float deltaTime = 0.016f; // 약 60FPS 기준
    spawnTimer += deltaTime;
    
    if (spawnTimer >= spawnInterval) {
        // 직접 GameWorld에 추가하지 않고 대기열에 추가
        SpawnObstacle();
        spawnTimer = 0.0f;
    }
}

void ObstacleSpawner::SpawnObstacle()
{
    std::cout << "ObstacleSpawner: 새로운 장애물 생성 대기열에 추가!" << std::endl;
    
    // 새로운 장애물 생성
    auto obstacle = std::make_unique<TempObstacle>();
    
    // GameWorld의 대기열에 추가 (직접 추가하지 않음)
    g_gameWorld.AddPendingObject(std::move(obstacle));
}