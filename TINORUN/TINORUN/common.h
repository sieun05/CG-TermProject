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
