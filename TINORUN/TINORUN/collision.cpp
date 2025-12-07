#include "common.h"
#include "game_object.h"
#include "tino.h"
#include "obstacle.h"
#include <algorithm>

// Tino�� Obstacle�� Ư�� ��ȯ�� ������ AABB ��ȯ �Լ�
AABB BoundaryToAABB(const GameObject* obj)
{
    const Boundary& boundary = obj->boundary;
    glm::vec3 position = obj->position;
    glm::vec3 rotation = obj->rotation;
    glm::vec3 scale = obj->scale;
    
    // boundary�� 6�� �����κ��� ������ 8�� �ڽ� �������� ���
    std::vector<glm::vec3> boundaryPoints = {
        boundary.r1,                                                    // ���� �Ʒ� ��
        boundary.r2,                                                    // ������ �Ʒ� ��  
        boundary.r3,                                                    // ������ �� ��
        boundary.r4,                                                    // ���� �� ��
        boundary.r5,                                                    // ���� �Ʒ� ��
        glm::vec3(boundary.r6.x, boundary.r5.y, boundary.r5.z),       // ������ �Ʒ� ��
        boundary.r6,                                                    // ������ �� ��
        glm::vec3(boundary.r5.x, boundary.r6.y, boundary.r6.z)        // ���� �� ��
    };
    
    // �⺻ ��ȯ ��� ����
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    
    // ��ü�� �߰� ��ȯ ����
    const Tino* tino = dynamic_cast<const Tino*>(obj);
    const Obstacle* obstacle = dynamic_cast<const Obstacle*>(obj);
    
    if (tino) {
        // Tino�� �߰� ��ȯ (Draw �Լ��� �����ϰ�)
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, 0.0f, 0.0f));
        model = model * rotate;
        model = translate * model;
    }
    else if (obstacle) {
        // Obstacle Ÿ�Ժ� �߰� ��ȯ
        if (obstacle->GetType() == Obstacle::ObstacleType::CACTUS) {
            glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = model * rotate;
        }
    }
    
    // ��� ���� ���� ��ǥ��� ��ȯ
    std::vector<glm::vec3> transformedPoints;
    for (const auto& point : boundaryPoints) {
        glm::vec4 worldPoint = model * glm::vec4(point, 1.0f);
        transformedPoints.push_back(glm::vec3(worldPoint));
    }
    
    // ��ȯ�� ����κ��� AABB ���
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

// ���� �Լ��� ���� (���� ȣȯ��)
AABB BoundaryToAABB(const Boundary& boundary, const glm::vec3& position, 
                    const glm::vec3& rotation, const glm::vec3& scale)
{
    // boundary�� 6�� �����κ��� ������ 8�� �ڽ� �������� ���
    std::vector<glm::vec3> boundaryPoints = {
        boundary.r1,                                                    // ���� �Ʒ� ��
        boundary.r2,                                                    // ������ �Ʒ� ��  
        boundary.r3,                                                    // ������ �� ��
        boundary.r4,                                                    // ���� �� ��
        boundary.r5,                                                    // ���� �Ʒ� ��
        glm::vec3(boundary.r6.x, boundary.r5.y, boundary.r5.z),       // ������ �Ʒ� ��
        boundary.r6,                                                    // ������ �� ��
        glm::vec3(boundary.r5.x, boundary.r6.y, boundary.r6.z)        // ���� �� ��
    };
    
    // ��ȯ ��� ���� (GameObject�� ���� ������)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    
    // ��� ���� ���� ��ǥ��� ��ȯ
    std::vector<glm::vec3> transformedPoints;
    for (const auto& point : boundaryPoints) {
        glm::vec4 worldPoint = model * glm::vec4(point, 1.0f);
        transformedPoints.push_back(glm::vec3(worldPoint));
    }
    
    // ��ȯ�� ����κ��� AABB ���
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

// �� ��Ȯ�� AABB �浹 �˻� �Լ� (��� ���� �߰�)
bool AABBIntersect(const AABB& box1, const AABB& box2, float tolerance)
{
    // ��� ������ �����Ͽ� �� ��Ȯ�� �浹 �˻�
    // X�࿡�� ��ġ���� Ȯ��
    bool xOverlap = (box1.max.x - tolerance) > (box2.min.x + tolerance) && 
                    (box1.min.x + tolerance) < (box2.max.x - tolerance);
    
    // Y�࿡�� ��ġ���� Ȯ��  
    bool yOverlap = (box1.max.y - tolerance) > (box2.min.y + tolerance) && 
                    (box1.min.y + tolerance) < (box2.max.y - tolerance);
    
    // Z�࿡�� ��ġ���� Ȯ��
    bool zOverlap = (box1.max.z - tolerance) > (box2.min.z + tolerance) && 
                    (box1.min.z + tolerance) < (box2.max.z - tolerance);
    
    // ��� �࿡�� ���ľ� �浹
    return xOverlap && yOverlap && zOverlap;
}

// ���� �Լ��� ���� (���� ȣȯ��)
bool AABBIntersect(const AABB& box1, const AABB& box2)
{
    return AABBIntersect(box1, box2, 0.01f);
}

// ������ GameObject �浹 �˻� �Լ�
bool CheckCollision(const GameObject* obj1, const GameObject* obj2)
{
    if (!obj1 || !obj2) {
        return false;
    }
    
    // ��Ȱ�� ��ü�� �浹���� ����
    if (!obj1->isActive || !obj2->isActive) {
        return false;
    }
    
    // �� ��ü�� boundary�� ��Ȯ�� AABB�� ��ȯ
    AABB aabb1 = BoundaryToAABB(obj1);
    AABB aabb2 = BoundaryToAABB(obj2);
    
    // ����� ��� (�ʿ�� �ּ� ����)
    // PrintAABB(aabb1, "Object1");
    // PrintAABB(aabb2, "Object2");
    
    // �� ��Ȯ�� AABB �浹 �˻� (��� ���� ����)
    return AABBIntersect(aabb1, aabb2, 0.1f); // 0.1f ��� ������ �� ������ �浹 �˻�
}

// ����׿� AABB ���� ��� �Լ�
void PrintAABB(const AABB& aabb, const std::string& name)
{
    std::cout << name << " AABB - Min: (" << aabb.min.x << ", " << aabb.min.y << ", " << aabb.min.z 
              << "), Max: (" << aabb.max.x << ", " << aabb.max.y << ", " << aabb.max.z << ")" << std::endl;
    
    // AABB ũ�� ������ ���
    glm::vec3 size = aabb.max - aabb.min;
    std::cout << name << " Size: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
}

// �浹 �ý��� �׽�Ʈ �Լ�
void TestCollisionSystem()
{
    std::cout << "\n=== �浹 �ý��� �׽�Ʈ ���� ===" << std::endl;
    
    // �׽�Ʈ�� AABB ���� (�� �������� �����)
    AABB box1(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 2.0f, 1.0f));
    AABB box2(glm::vec3(0.9f, 0.0f, -1.0f), glm::vec3(2.9f, 2.0f, 1.0f));  // �ణ ��ħ
    AABB box3(glm::vec3(1.2f, 0.0f, -1.0f), glm::vec3(3.2f, 2.0f, 1.0f));  // ���� ��ġ�� ����
    AABB box4(glm::vec3(3.0f, 0.0f, -1.0f), glm::vec3(5.0f, 2.0f, 1.0f));  // ������ �и�
    
    PrintAABB(box1, "Box1");
    PrintAABB(box2, "Box2");
    PrintAABB(box3, "Box3");
    PrintAABB(box4, "Box4");
    
    std::cout << "\n=== �浹 �׽�Ʈ ��� ===" << std::endl;
    std::cout << "Box1�� Box2 �浹 (������ 0.01): " << (AABBIntersect(box1, box2, 0.01f) ? "��" : "�ƴϿ�") << std::endl;
    std::cout << "Box1�� Box2 �浹 (������ 0.1): " << (AABBIntersect(box1, box2, 0.1f) ? "��" : "�ƴϿ�") << std::endl;
    std::cout << "Box1�� Box3 �浹 (������ 0.01): " << (AABBIntersect(box1, box3, 0.01f) ? "��" : "�ƴϿ�") << std::endl;
    std::cout << "Box1�� Box3 �浹 (������ 0.1): " << (AABBIntersect(box1, box3, 0.1f) ? "��" : "�ƴϿ�") << std::endl;
    std::cout << "Box1�� Box4 �浹 (������ 0.1): " << (AABBIntersect(box1, box4, 0.1f) ? "��" : "�ƴϿ�") << std::endl;
    
    std::cout << "=== �浹 �ý��� �׽�Ʈ �Ϸ� ===" << std::endl;
}