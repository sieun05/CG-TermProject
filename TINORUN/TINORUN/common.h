#pragma once
#include "Header.h"
#include <vector>
#include <string>
#include <sstream>

// ���� ����ü
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;      // �÷� �߰�
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

// ���� ����
class GameObject;

// �� GameObject�� �浹�� �˻��ϴ� �Լ�
bool CheckCollision(const GameObject* obj1, const GameObject* obj2);

// AABB (Axis-Aligned Bounding Box) ����ü
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint) 
        : min(minPoint), max(maxPoint) {}
};

// GameObject ��ü�κ��� ��Ȯ�� AABB�� ����ϴ� �Լ� (������ ����)
AABB BoundaryToAABB(const GameObject* obj);

// Boundary���� AABB�� ��ȯ�ϴ� �Լ� (���� ����, ���� ȣȯ��)
AABB BoundaryToAABB(const Boundary& boundary, const glm::vec3& position, 
                    const glm::vec3& rotation, const glm::vec3& scale);

// �� AABB�� �浹�� �˻��ϴ� �Լ� (��� ���� ����)
bool AABBIntersect(const AABB& box1, const AABB& box2, float tolerance);

// �� AABB�� �浹�� �˻��ϴ� �Լ� (�⺻ ��� ����)
bool AABBIntersect(const AABB& box1, const AABB& box2);

// ����׿� AABB ���� ��� �Լ�
void PrintAABB(const AABB& aabb, const std::string& name);

// �浹 �ý��� �׽�Ʈ �Լ�
void TestCollisionSystem();

// 바운더리 박스 표시 여부
extern bool showBoundaryBox;
