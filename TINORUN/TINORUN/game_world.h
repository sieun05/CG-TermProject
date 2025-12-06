#pragma once
#include "Header.h"
#include "game_object.h"
#include <memory>
#include <vector>

class GameWorld {
public:
    GameWorld() = default;
    ~GameWorld() = default;

    // 게임 객체 추가 (unique_ptr 사용으로 메모리 자동 관리)
    void AddObject(std::unique_ptr<GameObject> object);
    
    // 대기열에 객체 추가 (Update 중 안전하게 추가)
    void AddPendingObject(std::unique_ptr<GameObject> object);
    
    // 대기열의 객체들을 실제 게임 객체 리스트에 추가
    void ProcessPendingObjects();
    
    // 게임 객체 제거 (비활성화된 객체들 제거)
    void RemoveInactiveObjects();
    
    // 모든 객체 업데이트
    void UpdateAll();
    
    // 모든 객체 렌더링
    void DrawAll(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc);
    
    // 모든 객체 제거
    void Clear();
    
    // 활성 객체 수 반환
    size_t GetActiveObjectCount() const;

private:
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::vector<std::unique_ptr<GameObject>> pendingObjects; // 대기열 추가
};

// 전역 게임 월드 인스턴스
extern GameWorld g_gameWorld;