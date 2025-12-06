#include "common.h"
#include "game_object.h"
#include "tino.h"
#include "obstacle.h"
#include <algorithm>

// Tino와 Obstacle의 특수 변환을 고려한 AABB 변환 함수
AABB BoundaryToAABB(const GameObject* obj)
{
    const Boundary& boundary = obj->boundary;
    glm::vec3 position = obj->position;
    glm::vec3 rotation = obj->rotation;
    glm::vec3 scale = obj->scale;
    
    // boundary의 6개 점으로부터 완전한 8개 박스 꼭짓점을 계산
    std::vector<glm::vec3> boundaryPoints = {
        boundary.r1,                                                    // 왼쪽 아래 뒤
        boundary.r2,                                                    // 오른쪽 아래 뒤  
        boundary.r3,                                                    // 오른쪽 위 뒤
        boundary.r4,                                                    // 왼쪽 위 뒤
        boundary.r5,                                                    // 왼쪽 아래 앞
        glm::vec3(boundary.r6.x, boundary.r5.y, boundary.r5.z),       // 오른쪽 아래 앞
        boundary.r6,                                                    // 오른쪽 위 앞
        glm::vec3(boundary.r5.x, boundary.r6.y, boundary.r6.z)        // 왼쪽 위 앞
    };
    
    // 기본 변환 행렬 생성
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    
    // 객체별 추가 변환 적용
    const Tino* tino = dynamic_cast<const Tino*>(obj);
    const Obstacle* obstacle = dynamic_cast<const Obstacle*>(obj);
    
    if (tino) {
        // Tino의 추가 변환 (Draw 함수와 동일하게)
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, 0.0f, 0.0f));
        model = model * rotate;
        model = translate * model;
    }
    else if (obstacle) {
        // Obstacle 타입별 추가 변환
        if (obstacle->GetType() == Obstacle::ObstacleType::CACTUS) {
            glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = model * rotate;
        }
    }
    
    // 모든 점을 월드 좌표계로 변환
    std::vector<glm::vec3> transformedPoints;
    for (const auto& point : boundaryPoints) {
        glm::vec4 worldPoint = model * glm::vec4(point, 1.0f);
        transformedPoints.push_back(glm::vec3(worldPoint));
    }
    
    // 변환된 점들로부터 AABB 계산
    if (transformedPoints.empty()) {
        return AABB(glm::vec3(0.0f), glm::vec3(0.0f));
    }
    
    glm::vec3 minPoint = transformedPoints[0];
    glm::vec3 maxPoint = transformedPoints[0];
    
    for (const auto& point : transformedPoints) {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        minPoint.z = std::min(minPoint.z, point.z);
        
        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
        maxPoint.z = std::max(maxPoint.z, point.z);
    }
    
    return AABB(minPoint, maxPoint);
}

// 기존 함수도 유지 (하위 호환성)
AABB BoundaryToAABB(const Boundary& boundary, const glm::vec3& position, 
                    const glm::vec3& rotation, const glm::vec3& scale)
{
    // boundary의 6개 점으로부터 완전한 8개 박스 꼭짓점을 계산
    std::vector<glm::vec3> boundaryPoints = {
        boundary.r1,                                                    // 왼쪽 아래 뒤
        boundary.r2,                                                    // 오른쪽 아래 뒤  
        boundary.r3,                                                    // 오른쪽 위 뒤
        boundary.r4,                                                    // 왼쪽 위 뒤
        boundary.r5,                                                    // 왼쪽 아래 앞
        glm::vec3(boundary.r6.x, boundary.r5.y, boundary.r5.z),       // 오른쪽 아래 앞
        boundary.r6,                                                    // 오른쪽 위 앞
        glm::vec3(boundary.r5.x, boundary.r6.y, boundary.r6.z)        // 왼쪽 위 앞
    };
    
    // 변환 행렬 생성 (GameObject와 같은 순서로)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    
    // 모든 점을 월드 좌표계로 변환
    std::vector<glm::vec3> transformedPoints;
    for (const auto& point : boundaryPoints) {
        glm::vec4 worldPoint = model * glm::vec4(point, 1.0f);
        transformedPoints.push_back(glm::vec3(worldPoint));
    }
    
    // 변환된 점들로부터 AABB 계산
    if (transformedPoints.empty()) {
        return AABB(glm::vec3(0.0f), glm::vec3(0.0f));
    }
    
    glm::vec3 minPoint = transformedPoints[0];
    glm::vec3 maxPoint = transformedPoints[0];
    
    for (const auto& point : transformedPoints) {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        minPoint.z = std::min(minPoint.z, point.z);
        
        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
        maxPoint.z = std::max(maxPoint.z, point.z);
    }
    
    return AABB(minPoint, maxPoint);
}

// 더 정확한 AABB 충돌 검사 함수 (허용 오차 추가)
bool AABBIntersect(const AABB& box1, const AABB& box2, float tolerance = 0.01f)
{
    // 허용 오차를 적용하여 더 정확한 충돌 검사
    // X축에서 겹치는지 확인
    bool xOverlap = (box1.max.x - tolerance) > (box2.min.x + tolerance) && 
                    (box1.min.x + tolerance) < (box2.max.x - tolerance);
    
    // Y축에서 겹치는지 확인  
    bool yOverlap = (box1.max.y - tolerance) > (box2.min.y + tolerance) && 
                    (box1.min.y + tolerance) < (box2.max.y - tolerance);
    
    // Z축에서 겹치는지 확인
    bool zOverlap = (box1.max.z - tolerance) > (box2.min.z + tolerance) && 
                    (box1.min.z + tolerance) < (box2.max.z - tolerance);
    
    // 모든 축에서 겹쳐야 충돌
    return xOverlap && yOverlap && zOverlap;
}

// 기존 함수도 유지 (하위 호환성)
bool AABBIntersect(const AABB& box1, const AABB& box2)
{
    return AABBIntersect(box1, box2, 0.01f);
}

// 개선된 GameObject 충돌 검사 함수
bool CheckCollision(const GameObject* obj1, const GameObject* obj2)
{
    if (!obj1 || !obj2) {
        return false;
    }
    
    // 비활성 객체는 충돌하지 않음
    if (!obj1->isActive || !obj2->isActive) {
        return false;
    }
    
    // 각 객체의 boundary를 정확한 AABB로 변환
    AABB aabb1 = BoundaryToAABB(obj1);
    AABB aabb2 = BoundaryToAABB(obj2);
    
    // 디버그 출력 (필요시 주석 해제)
    // PrintAABB(aabb1, "Object1");
    // PrintAABB(aabb2, "Object2");
    
    // 더 정확한 AABB 충돌 검사 (허용 오차 적용)
    return AABBIntersect(aabb1, aabb2, 0.1f); // 0.1f 허용 오차로 더 엄격한 충돌 검사
}

// 디버그용 AABB 정보 출력 함수
void PrintAABB(const AABB& aabb, const std::string& name)
{
    std::cout << name << " AABB - Min: (" << aabb.min.x << ", " << aabb.min.y << ", " << aabb.min.z 
              << "), Max: (" << aabb.max.x << ", " << aabb.max.y << ", " << aabb.max.z << ")" << std::endl;
    
    // AABB 크기 정보도 출력
    glm::vec3 size = aabb.max - aabb.min;
    std::cout << name << " Size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
}

// 충돌 시스템 테스트 함수
void TestCollisionSystem()
{
    std::cout << "\n=== 충돌 시스템 테스트 시작 ===" << std::endl;
    
    // 테스트용 AABB 생성 (더 현실적인 값들로)
    AABB box1(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 2.0f, 1.0f));
    AABB box2(glm::vec3(0.9f, 0.0f, -1.0f), glm::vec3(2.9f, 2.0f, 1.0f));  // 약간 겹침
    AABB box3(glm::vec3(1.2f, 0.0f, -1.0f), glm::vec3(3.2f, 2.0f, 1.0f));  // 거의 겹치지 않음
    AABB box4(glm::vec3(3.0f, 0.0f, -1.0f), glm::vec3(5.0f, 2.0f, 1.0f));  // 완전히 분리
    
    PrintAABB(box1, "Box1");
    PrintAABB(box2, "Box2");
    PrintAABB(box3, "Box3");
    PrintAABB(box4, "Box4");
    
    std::cout << "\n=== 충돌 테스트 결과 ===" << std::endl;
    std::cout << "Box1과 Box2 충돌 (허용오차 0.01): " << (AABBIntersect(box1, box2, 0.01f) ? "예" : "아니오") << std::endl;
    std::cout << "Box1과 Box2 충돌 (허용오차 0.1): " << (AABBIntersect(box1, box2, 0.1f) ? "예" : "아니오") << std::endl;
    std::cout << "Box1과 Box3 충돌 (허용오차 0.01): " << (AABBIntersect(box1, box3, 0.01f) ? "예" : "아니오") << std::endl;
    std::cout << "Box1과 Box3 충돌 (허용오차 0.1): " << (AABBIntersect(box1, box3, 0.1f) ? "예" : "아니오") << std::endl;
    std::cout << "Box1과 Box4 충돌 (허용오차 0.1): " << (AABBIntersect(box1, box4, 0.1f) ? "예" : "아니오") << std::endl;
    
    std::cout << "=== 충돌 시스템 테스트 완료 ===" << std::endl;
}