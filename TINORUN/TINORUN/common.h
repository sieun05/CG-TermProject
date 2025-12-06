#pragma once
#include "Header.h"
#include <vector>
#include <string>
#include <sstream>

// 정점 구조체
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;      // 컬러 추가
    glm::vec2 texCoord;
    glm::vec3 normal;
};

inline std::vector<std::string> Split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// 전방 선언
class GameObject;

// 두 GameObject의 충돌을 검사하는 함수
bool CheckCollision(const GameObject* obj1, const GameObject* obj2);

// AABB (Axis-Aligned Bounding Box) 구조체
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint) 
        : min(minPoint), max(maxPoint) {}
};

// GameObject 객체로부터 정확한 AABB를 계산하는 함수 (개선된 버전)
AABB BoundaryToAABB(const GameObject* obj);

// Boundary에서 AABB로 변환하는 함수 (기존 버전, 하위 호환성)
AABB BoundaryToAABB(const Boundary& boundary, const glm::vec3& position, 
                    const glm::vec3& rotation, const glm::vec3& scale);

// 두 AABB의 충돌을 검사하는 함수 (허용 오차 포함)
bool AABBIntersect(const AABB& box1, const AABB& box2, float tolerance);

// 두 AABB의 충돌을 검사하는 함수 (기본 허용 오차)
bool AABBIntersect(const AABB& box1, const AABB& box2);

// 디버그용 AABB 정보 출력 함수
void PrintAABB(const AABB& aabb, const std::string& name);

// 충돌 시스템 테스트 함수
void TestCollisionSystem();
