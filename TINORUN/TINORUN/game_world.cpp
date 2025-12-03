#include "game_world.h"

// 전역 게임 월드 인스턴스 정의
GameWorld g_gameWorld;

void GameWorld::AddObject(std::unique_ptr<GameObject> object) {
    if (object) {
        gameObjects.push_back(std::move(object));
    }
}

void GameWorld::RemoveInactiveObjects() {
    // remove_if와 erase를 사용하여 비활성화된 객체들 제거
    gameObjects.erase(
        std::remove_if(gameObjects.begin(), gameObjects.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                return !obj->isActive;
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