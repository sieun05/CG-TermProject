#include "game_world.h"
#include "temp_obstacle.h"
#include "game_state.h"

// 전역 게임 월드 인스턴스 정의
GameWorld g_gameWorld;
GameState scene = GameState::TITLE;

void GameWorld::AddObject(std::unique_ptr<GameObject> object) {
    if (object) {
        gameObjects.push_back(std::move(object));
    }
}

void GameWorld::RemoveInactiveObjects() {
    // remove_if와 erase를 사용하여 비활성화된 객체들 제거
    // TempObstacle의 경우 ShouldBeRemoved() 조건도 확인
    gameObjects.erase(
        std::remove_if(gameObjects.begin(), gameObjects.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                if (!obj->isActive) {
                    return true;
                }
                
                // TempObstacle인 경우 추가 제거 조건 확인
                TempObstacle* tempObstacle = dynamic_cast<TempObstacle*>(obj.get());
                if (tempObstacle && tempObstacle->ShouldBeRemoved()) {
                    std::cout << "장애물 제거: x = " << tempObstacle->position.x << std::endl;
                    return true;
                }
                
                return false;
            }),
        gameObjects.end()
    );
}

void GameWorld::UpdateAll() {
    // 활성화된 모든 객체 업데이트
    for (auto& object : gameObjects) {
        if (object && object->isActive) {
            object->Update();
        }
    }
    
    // 비활성화된 객체들 제거
    RemoveInactiveObjects();
}

void GameWorld::DrawAll(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) {
    // 활성화된 모든 객체 렌더링
    for (auto& object : gameObjects) {
        if (object && object->isActive) {
            object->Draw(gProjection, gView, uMVP_loc);
        }
    }
}

void GameWorld::Clear() {
    gameObjects.clear();
}

size_t GameWorld::GetActiveObjectCount() const {
    size_t count = 0;
    for (const auto& object : gameObjects) {
        if (object && object->isActive) {
            count++;
        }
    }
    return count;
}