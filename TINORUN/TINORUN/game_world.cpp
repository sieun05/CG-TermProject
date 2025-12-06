#include "game_world.h"
#include "obstacle.h"
#include "game_state.h"
#include "tino.h"
#include "common.h"

// 전역 게임 월드 인스턴스 정의
GameWorld g_gameWorld;
GameState scene;

void GameWorld::AddObject(std::unique_ptr<GameObject> object) {
    if (object) {
        gameObjects.push_back(std::move(object));
    }
}

void GameWorld::AddPendingObject(std::unique_ptr<GameObject> object) {
    if (object) {
        pendingObjects.push_back(std::move(object));
    }
}

void GameWorld::ProcessPendingObjects() {
    // 대기 중인 객체들을 메인 게임 객체 리스트에 추가
    for (auto& obj : pendingObjects) {
        if (obj) {
            gameObjects.push_back(std::move(obj));
        }
    }
    pendingObjects.clear();
}

void GameWorld::RemoveInactiveObjects() {
    // remove_if와 erase를 사용하여 비활성화된 객체들 제거
    // TempObstacle에 대해 ShouldBeRemoved() 조건도 확인
    gameObjects.erase(
        std::remove_if(gameObjects.begin(), gameObjects.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                if (!obj->isActive) {
                    return true;
                }

                Obstacle* obstacle = dynamic_cast<Obstacle*>(obj.get());
                if (obstacle && obstacle->ShouldBeRemoved()) {
                    std::cout << "장애물 제거: x = " << obstacle->position.x << std::endl;
                    return true;
                }

                return false;
            }),
        gameObjects.end()
    );
}

void GameWorld::CheckCollisions() {
    // Tino 객체 찾기
    Tino* tino = nullptr;
    for (auto& obj : gameObjects) {
        if (obj && obj->isActive) {
            tino = dynamic_cast<Tino*>(obj.get());
            if (tino) break;
        }
    }
    
    if (!tino) return; // Tino가 없으면 충돌 검사 생략
    
    // Tino와 다른 모든 객체 간의 충돌 검사
    for (auto& obj : gameObjects) {
        if (!obj || !obj->isActive || obj.get() == tino) {
            continue; // 비활성 객체나 Tino 자기 자신은 건너뜀
        }
        
        // 장애물과의 충돌 검사
        Obstacle* obstacle = dynamic_cast<Obstacle*>(obj.get());
        if (obstacle) {
            if (CheckCollision(tino, obstacle)) {
                // 충돌 발생!
               /* std::cout << "충돌 감지: Tino와 " << 
                    (obstacle->GetType() == Obstacle::ObstacleType::CACTUS ? "선인장" :
                     obstacle->GetType() == Obstacle::ObstacleType::TREE ? "나무" :
                     obstacle->GetType() == Obstacle::ObstacleType::MUSHROOM ? "버섯" :
                     obstacle->GetType() == Obstacle::ObstacleType::BIRD ? "새" : "장애물")
                    << " 충돌!" << std::endl;*/
                
                // 충돌 콜백 호출
                tino->OnCollision(obstacle);
                obstacle->OnCollision(tino);
                
                // 게임 오버 처리 (선택적)
                // scene = GameState::GAME_OVER;
            }
        }
    }
}

void GameWorld::UpdateAll() {
    // 활성화된 모든 객체 업데이트
    for (auto& object : gameObjects) {
        if (object && object->isActive) {
            object->Update();
        }
    }
    
    // 충돌 검사 수행
    CheckCollisions();
    
    // Update 이후에 대기 중인 새로운 객체들 추가
    ProcessPendingObjects();
    
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
    pendingObjects.clear(); // 대기 객체들도 클리어
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