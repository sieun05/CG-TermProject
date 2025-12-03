#include "tino.h"
#include <fstream>
#include <sstream>

// STB 이미지 라이브러리 대신 임시로 텍스처 없이 구현
Tino::Tino(const std::string& objPath, const std::string& texturePath)
    : VAO(0), VBO(0), EBO(0), textureID(0), isLoaded(false)
{
    std::cout << "Tino 생성 시도, OBJ 경로: " << objPath << std::endl;
    
    if (LoadOBJ(objPath)) {
        std::cout << "OBJ 로딩 성공!" << std::endl;
        // 텍스처는 나중에 구현하고 우선 컬러만 사용
        SetupMesh();
        isLoaded = true;
        std::cout << "Tino 초기화 완료" << std::endl;
    } else {
        std::cerr << "Tino 초기화 실패: OBJ 로딩 실패" << std::endl;
    }
}

Tino::~Tino()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    if (textureID != 0) glDeleteTextures(1, &textureID);
}

bool Tino::LoadOBJ(const std::string& objPath)
{
    std::cout << "OBJ 파일 로딩 시작: " << objPath << std::endl;
    
    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;
        
        // 다른 경로들도 시도해보기
        std::string altPath1 = "assets/Tino.obj";
        std::string altPath2 = "../assets/Tino.obj";
        std::string altPath3 = "./TINORUN/assets/Tino.obj";
        
        std::cout << "대체 경로 시도: " << altPath1 << std::endl;
        file.open(altPath1);
        if (!file.is_open()) {
            std::cout << "대체 경로 시도: " << altPath2 << std::endl;
            file.open(altPath2);
            if (!file.is_open()) {
                std::cout << "대체 경로 시도: " << altPath3 << std::endl;
                file.open(altPath3);
                if (!file.is_open()) {
                    std::cout << "OBJ 파일을 찾을 수 없어서 테스트 큐브를 생성합니다." << std::endl;
                    return CreateTestCube();
                }
            }
        }
    }

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<glm::vec3> temp_normals;
    std::vector<unsigned int> vertexIndices, texCoordIndices, normalIndices;

    std::string line;
    int vertexCount = 0, faceCount = 0;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // 정점 위치
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
            vertexCount++;
        }
        else if (prefix == "vt") {
            // 텍스처 좌표
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            temp_texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // 법선 벡터
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            // 면 정보
            std::string vertex1, vertex2, vertex3;
            iss >> vertex1 >> vertex2 >> vertex3;
            faceCount++;
            
            // "v/vt/vn" 형식 파싱
            auto parseVertex = [](const std::string& vertexStr) {
                std::vector<std::string> parts = Split(vertexStr, '/');
                std::vector<unsigned int> indices;
                for (const auto& part : parts) {
                    if (!part.empty()) {
                        indices.push_back(std::stoi(part) - 1); // OBJ는 1-based 인덱스
                    } else {
                        indices.push_back(0); // 빈 부분은 0으로 처리
                    }
                }
                return indices;
            };

            auto v1_indices = parseVertex(vertex1);
            auto v2_indices = parseVertex(vertex2);
            auto v3_indices = parseVertex(vertex3);

            // 각 정점의 인덱스 저장
            if (v1_indices.size() >= 1) vertexIndices.push_back(v1_indices[0]);
            if (v1_indices.size() >= 2) texCoordIndices.push_back(v1_indices[1]);
            if (v1_indices.size() >= 3) normalIndices.push_back(v1_indices[2]);

            if (v2_indices.size() >= 1) vertexIndices.push_back(v2_indices[0]);
            if (v2_indices.size() >= 2) texCoordIndices.push_back(v2_indices[1]);
            if (v2_indices.size() >= 3) normalIndices.push_back(v2_indices[2]);

            if (v3_indices.size() >= 1) vertexIndices.push_back(v3_indices[0]);
            if (v3_indices.size() >= 2) texCoordIndices.push_back(v3_indices[1]);
            if (v3_indices.size() >= 3) normalIndices.push_back(v3_indices[2]);
        }
    }

    file.close();
    
    std::cout << "OBJ 파일 파싱 완료: 정점 " << vertexCount << "개, 면 " << faceCount << "개" << std::endl;

    // 인덱스를 사용해서 최종 정점 배열 구성
    vertices.clear();
    indices.clear();

    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        Vertex vertex;
        
        // 위치
        if (vertexIndices[i] < temp_vertices.size()) {
            vertex.position = temp_vertices[vertexIndices[i]];
        }
        
        // 기본 파란색 컬러 설정 (Tino 캐릭터 색상)
        vertex.color = glm::vec3(0.3f, 0.7f, 1.0f);
        
        // 텍스처 좌표
        if (i < texCoordIndices.size() && texCoordIndices[i] < temp_texCoords.size()) {
            vertex.texCoord = temp_texCoords[texCoordIndices[i]];
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }
        
        // 법선
        if (i < normalIndices.size() && normalIndices[i] < temp_normals.size()) {
            vertex.normal = temp_normals[normalIndices[i]];
        } else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(i));
    }

    std::cout << "최종 Loaded OBJ: " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;
    
    if (vertices.empty()) {
        std::cerr << "경고: 정점 데이터가 없습니다!" << std::endl;
        return false;
    }
    
    return true;
}

// 테스트용 큐브 생성 함수 추가
bool Tino::CreateTestCube() 
{
    std::cout << "테스트 큐브 생성 중..." << std::endl;
    
    vertices.clear();
    indices.clear();
    
    // 간단한 큐브 정점들
    vertices = {
        // 앞면
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        
        // 뒷면
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}}
    };
    
    indices = {
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
    
    std::cout << "테스트 큐브 생성 완료: " << vertices.size() << " 정점, " << indices.size() << " 인덱스" << std::endl;
    return true;
}

bool Tino::LoadTexture(const std::string& texturePath)
{
    // 텍스처 로딩은 나중에 구현
    std::cout << "Texture loading not implemented yet: " << texturePath << std::endl;
    return true;
}

void Tino::SetupMesh()
{
    std::cout << "SetupMesh 시작" << std::endl;
    
    // VAO 생성
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO 설정
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // EBO 설정
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // 정점 속성 설정
    // 위치 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 컬러 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // 텍스처 좌표 (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
    
    std::cout << "SetupMesh 완료, VAO: " << VAO << std::endl;
}

void Tino::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!isLoaded) {
        std::cerr << "Tino가 로드되지 않아서 그릴 수 없습니다" << std::endl;
        return;
    }

    // 텍스처 사용 모드 비활성화 (현재는 컬러만 사용)
    if (uUseTexture_loc >= 0) {
        glUniform1i(uUseTexture_loc, 0); // false
    }

    glBindVertexArray(VAO);

    // 모델 매트릭스 계산
    glm::mat4 model = GetModelMatrix();
    glm::mat4 mvp = gProjection * gView * model;

    // 유니폼 설정
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 채워진 삼각형으로 그리기
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    
    // 추가로 와이어프레임도 그려서 구조를 확인할 수 있게 함
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    
    // 다시 FILL 모드로 복원
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

void Tino::Update()
{
    // 필요한 업데이트 로직 구현
}

// 유틸리티 함수들
std::vector<std::string> Split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

GLuint LoadTextureFromFile(const std::string& path)
{
    // 텍스처 로딩은 나중에 구현
    return 0;
}