#include "obstacle.h"
#include "game_state.h"
#include "game_world.h"
#include "LoadBitmap.h"
#include <fstream>
#include <sstream>
#include <random>

// 정적 상수 정의
const float Obstacle::REMOVAL_X_POSITION = -20.0f;
const float Obstacle::SPAWN_X_POSITION = 40.0f;

// 장애물용 전역 변수 정의
GLuint VAO_obstacle = 0;
GLuint VBO_obstacle[2] = { 0, };
GLuint EBO_obstacle = 0;

// 기본 Obstacle 클래스 구현
Obstacle::Obstacle(const std::string& objPath, const std::string& texturePath)
    : VAO(0), VBO(0), EBO(0), textureID(0), moveSpeed(-6.0f), bmp(nullptr), isLoaded(false)
{
    position = glm::vec3(SPAWN_X_POSITION, 0.5f, 0.0f);
    scale = glm::vec3(0.25f, 0.25f, 0.25f);
	rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    // 충돌 영역 설정 (장애물의 기본 크기 기준)
    boundary.r1 = glm::vec3(-0.5f, -0.5f, -0.5f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(0.5f, -0.5f, -0.5f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(0.5f, 0.5f, -0.5f);   // 오른쪽 위 뒤
    boundary.r4 = glm::vec3(-0.5f, 0.5f, -0.5f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-0.5f, -0.5f, 0.5f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(0.5f, 0.5f, 0.5f);    // 오른쪽 위 앞

    //std::cout << "장애물 생성 시도, OBJ 경로: " << objPath << std::endl;

    if (LoadOBJ(objPath)) {
        //std::cout << "OBJ 로딩 성공!" << std::endl;
        SetupMesh();
        isLoaded = true;
        //std::cout << "장애물 초기화 완료" << std::endl;
    }
    else {
        std::cerr << "장애물 초기화 실패: OBJ 로딩 실패" << std::endl;
    }
    
    // 경계 박스 메시 설정
    SetupBoundaryMesh();
    
    if (LoadTexture(texturePath)) {
        //std::cout << "장애물 텍스처 로딩 성공!" << std::endl;
    }
    else {
        std::cerr << "장애물 텍스처 로딩 실패!" << std::endl;
    }
}

Obstacle::~Obstacle()
{
	if (VAO != 0) glDeleteVertexArrays(1, &VAO);
	if (VBO != 0) glDeleteBuffers(1, &VBO);
	if (EBO != 0) glDeleteBuffers(1, &EBO);
	if (textureID != 0) glDeleteTextures(1, &textureID);
	
	// 경계 박스 버퍼 정리
	if (boundaryVAO != 0) glDeleteVertexArrays(1, &boundaryVAO);
	if (boundaryVBO != 0) glDeleteBuffers(1, &boundaryVBO);
}

bool Obstacle::LoadOBJ(const std::string& objPath)
{
    //std::cout << "OBJ 파일 로딩 시작: " << objPath << std::endl;

    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;

        // 다른 경로들도 시도해보기
        std::string altPath1 = objPath;
        std::string altPath2 = ".." + objPath;
        std::string altPath3 = "./TINORUN/" + objPath;

        std::cout << "대체 경로 시도: " << altPath1 << std::endl;
        file.open(altPath1);
        if (!file.is_open()) {
            std::cout << "대체 경로 시도: " << altPath2 << std::endl;
            file.open(altPath2);
            if (!file.is_open()) {
                std::cout << "대체 경로 시도: " << altPath3 << std::endl;
                file.open(altPath3);
                if (!file.is_open()) {
                    std::cout << "OBJ 파일을 찾을 수 없음." << std::endl;
                    return false;
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
            std::vector<std::string> faceVertices;
            std::string vertexStr;
            while (iss >> vertexStr) {
                faceVertices.push_back(vertexStr);
            }

            if (faceVertices.size() < 3) continue; // 삼각형이 아닌 면은 무시
            faceCount++;

            // "v/vt/vn" 형식 파싱
            auto parseVertex = [](const std::string& vertexStr) {
                std::vector<std::string> parts = Split(vertexStr, '/');
                std::vector<unsigned int> indices;
                for (const auto& part : parts) {
                    if (!part.empty()) {
                        indices.push_back(std::stoi(part) - 1); // OBJ는 1-based 인덱스
                    }
                    else {
                        indices.push_back(0); // 빈 부분은 0으로 처리
                    }
                }
                return indices;
                };

            if (faceVertices.size() == 3) {
                for (int i = 0; i < 3; ++i) {
                    auto indices = parseVertex(faceVertices[i]);
                    if (indices.size() >= 1) vertexIndices.push_back(indices[0]);
                    if (indices.size() >= 2) texCoordIndices.push_back(indices[1]);
                    if (indices.size() >= 3) normalIndices.push_back(indices[2]);
                }
            }
            else if (faceVertices.size() == 4) {
                // 사각형을 두 개의 삼각형으로 분할
                int triangleIndices[6] = { 0, 1, 2, 0, 2, 3 };
                for (int i = 0; i < 6; ++i) {
                    auto indices = parseVertex(faceVertices[triangleIndices[i]]);
                    if (indices.size() >= 1) vertexIndices.push_back(indices[0]);
                    if (indices.size() >= 2) texCoordIndices.push_back(indices[1]);
                    if (indices.size() >= 3) normalIndices.push_back(indices[2]);
                }
            }
        }
    }

    file.close();

    //std::cout << "OBJ 파일 파싱 완료: 정점 " << vertexCount << "개, 면 " << faceCount << "개" << std::endl;

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
        }
        else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        // 법선
        if (i < normalIndices.size() && normalIndices[i] < temp_normals.size()) {
            vertex.normal = temp_normals[normalIndices[i]];
        }
        else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(i));
    }

    //std::cout << "최종 Loaded OBJ: " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;

    if (vertices.empty()) {
        std::cerr << "경고: 정점 데이터가 없습니다!" << std::endl;
        return false;
    }

    return true;
}
bool Obstacle::LoadTexture(const std::string& texturePath)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 텍스처 매개변수 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 이미지 로드 (여기서는 단색 이미지 사용)
    unsigned char* data = LoadDIBitmap(texturePath.c_str(), &bmp);
    if (data == NULL) {
        std::cerr << "Failed to load texture: " << texturePath << std::endl;
        return false;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp->bmiHeader.biWidth, bmp->bmiHeader.biHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return true;
}
void Obstacle::SetupMesh()
{
    std::cout << "obstacle SetupMesh 시작" << std::endl;

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

    std::cout << "obstacle SetupMesh 완료, VAO: " << VAO << std::endl;
}

bool Obstacle::ShouldBeRemoved() const
{
    return position.x <= REMOVAL_X_POSITION;
}

void Obstacle::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
}

void Obstacle::Update()
{
    // x축으로 이동 (약 60FPS 기준 deltaTime = 0.016f 가정)
    const float deltaTime = 0.016f;
    position.x += moveSpeed * deltaTime;

    // 디버그 출력 (가끔씩만)
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) { // 1초마다 출력
        //std::cout << "장애물 위치: x = " << position.x << ", y = " << position.y << std::endl;
    }
}

inline void Obstacle::OnCollision(GameObject* other) {
}


void Obstacle::SetupBoundaryMesh()
{
    // 박스의 8개 정점 정의 (boundary의 r1~r6를 이용해서 전체 8개 정점 구성)
    float boundaryVertices[] = {
        // 뒷면 4개 정점 (z = -0.5)
        boundary.r1.x, boundary.r1.y, boundary.r1.z, 0.0f, 1.0f, 0.0f,  // 0: 왼쪽 아래 뒤 (초록색)
        boundary.r2.x, boundary.r2.y, boundary.r2.z, 0.0f, 1.0f, 0.0f,  // 1: 오른쪽 아래 뒤
        boundary.r3.x, boundary.r3.y, boundary.r3.z, 0.0f, 1.0f, 0.0f,  // 2: 오른쪽 위 뒤
        boundary.r4.x, boundary.r4.y, boundary.r4.z, 0.0f, 1.0f, 0.0f,  // 3: 왼쪽 위 뒤
        
        // 앞면 4개 정점 (z = +0.5)
        boundary.r5.x, boundary.r5.y, boundary.r5.z, 0.0f, 1.0f, 0.0f,  // 4: 왼쪽 아래 앞
        boundary.r6.x, boundary.r5.y, boundary.r5.z, 0.0f, 1.0f, 0.0f,  // 5: 오른쪽 아래 앞
        boundary.r6.x, boundary.r6.y, boundary.r6.z, 0.0f, 1.0f, 0.0f,  // 6: 오른쪽 위 앞
        boundary.r5.x, boundary.r6.y, boundary.r6.z, 0.0f, 1.0f, 0.0f   // 7: 왼쪽 위 앞
    };

    glGenVertexArrays(1, &boundaryVAO);
    glGenBuffers(1, &boundaryVBO);

    glBindVertexArray(boundaryVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, boundaryVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundaryVertices), boundaryVertices, GL_STATIC_DRAW);

    // 위치 속성
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 색상 속성
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Obstacle::DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (boundaryVAO == 0) return;

    // 텍스처 사용 안함
    glUniform1i(uUseTexture_loc, 0);

    glBindVertexArray(boundaryVAO);

    // 현재 장애물의 변환 행렬 적용
    glm::mat4 model = GetModelMatrix();
    
    // 장애물 타입에 따른 추가 변환 적용
    if (GetType() == ObstacleType::CACTUS) {
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = model * rotate;
    }

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 선 두께 설정
    glLineWidth(2.0f);
    
    // 박스의 12개 모서리를 선으로 그리기
    unsigned int boundaryIndices[] = {
        // 뒷면의 4개 모서리
        0, 1,  1, 2,  2, 3,  3, 0,
        // 앞면의 4개 모서리  
        4, 5,  5, 6,  6, 7,  7, 4,
        // 앞뒤를 연결하는 4개 모서리
        0, 4,  1, 5,  2, 6,  3, 7
    };

    // 인덱스를 이용해 선 그리기
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boundaryIndices);
    
    // 선 두께 복원
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
}


// Cactus 구현
Cactus::Cactus() : Obstacle()
{
    // Cactus 전용 boundary 설정 (더 크고 높게)
    boundary.r1 = glm::vec3(-2.0f, 0.0f, -2.0f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(2.0f, 0.0f, -2.0f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(2.0f, 15.0f, -2.0f);   // 오른쪽 위 뒤
    boundary.r4 = glm::vec3(-2.0f, 15.0f, -2.0f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-2.0f, 0.0f, 2.0f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(2.0f, 15.0f, 2.0f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

Cactus::Cactus(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{

    // Cactus 전용 boundary 설정 (더 크고 높게)
    boundary.r1 = glm::vec3(-2.0f, 0.0f, -2.0f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(2.0f, 0.0f, -2.0f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(2.0f, 15.0f, -2.0f);   // 오른쪽 위 뒤
    boundary.r4 = glm::vec3(-2.0f, 15.0f, -2.0f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-2.0f, 0.0f, 2.0f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(2.0f, 15.0f, 2.0f);    // 오른쪽 위 앞

    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

void Cactus::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!isLoaded) return;

    glUniform1i(uUseTexture_loc, 1);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uTextureSampler_loc, 0);

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glm::mat4 model = GetModelMatrix();
    glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = model * rotate;

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // 경계 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Cactus::Update()
{
    Obstacle::Update(); // 기본 이동

    const float deltaTime = 0.016f;
}


// Tree 구현
Tree::Tree() : Obstacle()
{
    
    // Tree 전용 boundary 설정 (나무 모양에 맞게)
    boundary.r1 = glm::vec3(-3.0f, 8.0f, -3.0f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(3.0f, 8.0f, -3.0f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(3.0f, 16.0f, -3.0f);   // 오른쪽 위 뒤 (더 높게)
    boundary.r4 = glm::vec3(-3.0f, 16.0f, -3.0f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-3.0f, 8.0f, 3.0f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(3.0f, 16.0f, 3.0f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

Tree::Tree(const std::string& objPath, const std::string& texturePath) 
    : Obstacle(objPath, texturePath)
{
    moveSpeed = -6.0f;
    
    // 정점 색상을 빨간색으로 변경
    for (auto& vertex : vertices) {
        vertex.color = glm::vec3(1.0f, 0.2f, 0.2f); // 빨간색
    }
    if (isLoaded) {
        SetupMesh(); // 색상 변경 후 메시 재설정
    }
    
    // Tree 전용 boundary 설정 (나무 모양에 맞게)
    boundary.r1 = glm::vec3(-3.0f, 8.0f, -3.0f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(3.0f, 8.0f, -3.0f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(3.0f, 16.0f, -3.0f);   // 오른쪽 위 뒤 (더 높게)
    boundary.r4 = glm::vec3(-3.0f, 16.0f, -3.0f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-3.0f, 8.0f, 3.0f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(3.0f, 16.0f, 3.0f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

void Tree::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!isLoaded) return;

    glUniform1i(uUseTexture_loc, 1);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uTextureSampler_loc, 0);

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glm::mat4 model = GetModelMatrix();

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // 경계 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Tree::Update()
{
    Obstacle::Update(); // 기본 이동
    
    // 가시 회전
    const float deltaTime = 0.016f;
}

// Mushroom 구현
Mushroom::Mushroom() : Obstacle()
{
    // Mushroom 전용 boundary 설정 (버섯 모양에 맞게 - 작고 낮음)
    boundary.r1 = glm::vec3(-0.8f, 0.0f, -0.8f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(0.8f, 0.0f, -0.8f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(0.8f, 2.0f, -0.8f);   // 오른쪽 위 뒤 (낮게)
    boundary.r4 = glm::vec3(-0.8f, 2.0f, -0.8f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-0.8f, 0.0f, 0.8f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(0.8f, 2.0f, 0.8f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

Mushroom::Mushroom(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Mushroom 전용 boundary 설정 (버섯 모양에 맞게 - 작고 낮음)
    boundary.r1 = glm::vec3(-0.8f, 0.0f, -0.8f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(0.8f, 0.0f, -0.8f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(0.8f, 2.0f, -0.8f);   // 오른쪽 위 뒤 (낮게)
    boundary.r4 = glm::vec3(-0.8f, 2.0f, -0.8f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-0.8f, 0.0f, 0.8f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(0.8f, 2.0f, 0.8f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

void Mushroom::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!isLoaded) return;

    glUniform1i(uUseTexture_loc, 1);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uTextureSampler_loc, 0);

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glm::mat4 model = GetModelMatrix();

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // 경계 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Mushroom::Update()
{
    Obstacle::Update(); // 기본 이동
    
    const float deltaTime = 0.016f;
    
}

// Bird 구현
Bird::Bird() : Obstacle()
{
    moveSpeed = -6.0f;
    
    // Bird 전용 boundary 설정 (새 모양에 맞게 - 날개 포함)
    boundary.r1 = glm::vec3(-1.2f, -0.2f, -0.8f); // 왼쪽 아래 뒤 (날개 폭 고려)
    boundary.r2 = glm::vec3(1.2f, -0.2f, -0.8f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(1.2f, 1.4f, -0.8f);   // 오른쪽 위 뒤 (새 높이)
    boundary.r4 = glm::vec3(-1.2f, 1.4f, -0.8f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-1.2f, -0.2f, 0.8f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(1.2f, 1.4f, 0.8f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

Bird::Bird(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{
    position = glm::vec3(SPAWN_X_POSITION, 4.0f, 0.0f);
	scale = glm::vec3(0.7f, 0.7f, 0.7f);
	rotation = glm::vec3(0.0f, -180.0f, 0.0f);
    
    // Bird 전용 boundary 설정 (새 모양에 맞게 - 날개 포함)
    boundary.r1 = glm::vec3(-1.2f, -0.2f, -0.8f); // 왼쪽 아래 뒤 (날개 폭 고려)
    boundary.r2 = glm::vec3(1.2f, -0.2f, -0.8f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(1.2f, 1.4f, -0.8f);   // 오른쪽 위 뒤 (새 높이)
    boundary.r4 = glm::vec3(-1.2f, 1.4f, -0.8f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-1.2f, -0.2f, 0.8f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(1.2f, 1.4f, 0.8f);    // 오른쪽 위 앞
    
    // boundary 변경 후 boundary 메시 다시 설정
    SetupBoundaryMesh();
}

void Bird::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!isLoaded) return;

    glUniform1i(uUseTexture_loc, 1);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uTextureSampler_loc, 0);

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glm::mat4 model = GetModelMatrix();

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // 경계 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Bird::Update()
{
    Obstacle::Update(); // 기본 이동
    
    const float deltaTime = 0.016f;
}


ObstacleSpawner::ObstacleSpawner()
    : spawnTimer(0.0f), spawnInterval(2.0f)
{
    // 스포너는 렌더링되지 않는 객체이므로 위치는 상관없음
    std::cout << "ObstacleSpawner 생성됨" << std::endl;
}

void ObstacleSpawner::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    // 스포너는 렌더링되지 않음
}

void ObstacleSpawner::Update()
{
    // PLAYING 상태일 때만 장애물 생성
    if (scene != GameState::PLAYING) {
        spawnTimer = 0.0f; // 다른 상태에서는 타이머 초기화
        return;
    }

    const float deltaTime = 0.016f; // 약 60FPS 기준
    spawnTimer += deltaTime;

    if (spawnTimer >= spawnInterval) {
        // 직접 GameWorld에 추가하지 않고 대기열에 추가
        SpawnObstacle();
        spawnTimer = 0.0f;
    }
}

void ObstacleSpawner::SpawnObstacle()
{
    std::cout << "ObstacleSpawner: 새로운 장애물 생성 대기열에 추가!" << std::endl;

    // 랜덤하게 장애물 종류 선택
    auto obstacle = CreateRandomObstacle();
    if (obstacle) {
        g_gameWorld.AddPendingObject(std::move(obstacle));
    }
}

std::unique_ptr<Obstacle> ObstacleSpawner::CreateRandomObstacle()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 3);

    int obstacleType = dis(gen);

    switch (obstacleType) {
        case 0:
            std::cout << "선인장 생성" << std::endl;
            return std::make_unique<Cactus>("assets/obstacle1.obj", "assets/obstacle1_base.bmp");
        case 1:
            std::cout << "나무 생성" << std::endl;
            return std::make_unique<Tree>("assets/obstacle2.obj", "assets/obstacle2_base.bmp");
        case 2:
            std::cout << "버섯 생성" << std::endl;
            return std::make_unique<Mushroom>("assets/obstacle3.obj", "assets/obstacle3_base.bmp");
        case 3:
            std::cout << "새 생성" << std::endl;
            return std::make_unique<Bird>("assets/bird.obj", "assets/bird_base.bmp");
        default:
            return std::make_unique<Obstacle>("assets/obstacle1.obj", "assets/obstacle1_base.bmp");
    }
}
