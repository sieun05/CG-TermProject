#include "obstacle.h"
#include "game_state.h"
#include "game_world.h"
#include "LoadBitmap.h"
#include <fstream>
#include <sstream>
#include <random>

// ���� ��� ����
const float Obstacle::REMOVAL_X_POSITION = -20.0f;
const float Obstacle::SPAWN_X_POSITION = 40.0f;

// ��ֹ��� ���� ���� ����
GLuint VAO_obstacle = 0;
GLuint VBO_obstacle[2] = { 0, };
GLuint EBO_obstacle = 0;

// ���� ���� ���� (main.cpp���� ���ǵ�)
extern int gameScore;

// ���� �ӵ� ��� �Լ�
float GetObstacleSpeed() {
    int speedLevel = gameScore / 1000;  // 1000������ ������

    // �ӵ� ���� �˸� (�� ����)
    static int lastNotifiedLevel = -1;
    if (speedLevel != lastNotifiedLevel && speedLevel > 0) {
        std::cout << "�ӵ� ����! ���� " << speedLevel << " (����: " << gameScore << ")" << std::endl;
        lastNotifiedLevel = speedLevel;
    }

    // 기본 속도: -6.0f, 1000점마다 10% 증가, 최대 2.5배까지
    float speedMultiplier = 1.0f + (speedLevel * 0.2f);
    float speed = -6.0f * speedMultiplier;

    return speed;
}

// 기본 Obstacle 클래스 구현
Obstacle::Obstacle()
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

    // 경계 박스 메시 설정
    SetupBoundaryMesh();
}

Obstacle::Obstacle(const std::string& objPath, const std::string& texturePath)
    : VAO(0), VBO(0), EBO(0), textureID(0), moveSpeed(-6.0f), bmp(nullptr), isLoaded(false)
{
    position = glm::vec3(SPAWN_X_POSITION, 0.5f, 0.0f);
    scale = glm::vec3(0.25f, 0.25f, 0.25f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    // �浹 ���� ���� (��ֹ��� �⺻ ũ�� ����)
    boundary.r1 = glm::vec3(-0.5f, -0.5f, -0.5f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(0.5f, -0.5f, -0.5f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(0.5f, 0.5f, -0.5f);   // ������ �� ��
    boundary.r4 = glm::vec3(-0.5f, 0.5f, -0.5f);  // ���� �� ��
    boundary.r5 = glm::vec3(-0.5f, -0.5f, 0.5f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(0.5f, 0.5f, 0.5f);    // ������ �� ��

    //std::cout << "��ֹ� ���� �õ�, OBJ ���: " << objPath << std::endl;

    if (LoadOBJ(objPath)) {
        //std::cout << "OBJ �ε� ����!" << std::endl;
        SetupMesh();
        isLoaded = true;
        //std::cout << "��ֹ� �ʱ�ȭ �Ϸ�" << std::endl;
    }
    else {
        std::cerr << "��ֹ� �ʱ�ȭ ����: OBJ �ε� ����" << std::endl;
    }

    // ��� �ڽ� �޽� ����
    SetupBoundaryMesh();

    if (LoadTexture(texturePath)) {
        //std::cout << "��ֹ� �ؽ�ó �ε� ����!" << std::endl;
    }
    else {
        std::cerr << "��ֹ� �ؽ�ó �ε� ����!" << std::endl;
    }
}

Obstacle::~Obstacle()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (EBO != 0) glDeleteBuffers(1, &EBO);
    if (textureID != 0) glDeleteTextures(1, &textureID);

    // ��� �ڽ� ���� ����
    if (boundaryVAO != 0) glDeleteVertexArrays(1, &boundaryVAO);
    if (boundaryVBO != 0) glDeleteBuffers(1, &boundaryVBO);
}

bool Obstacle::LoadOBJ(const std::string& objPath)
{
    //std::cout << "OBJ ���� �ε� ����: " << objPath << std::endl;

    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;

        // �ٸ� ��ε鵵 �õ��غ���
        std::string altPath1 = objPath;
        std::string altPath2 = ".." + objPath;
        std::string altPath3 = "./TINORUN/" + objPath;

        std::cout << "��ü ��� �õ�: " << altPath1 << std::endl;
        file.open(altPath1);
        if (!file.is_open()) {
            std::cout << "��ü ��� �õ�: " << altPath2 << std::endl;
            file.open(altPath2);
            if (!file.is_open()) {
                std::cout << "��ü ��� �õ�: " << altPath3 << std::endl;
                file.open(altPath3);
                if (!file.is_open()) {
                    std::cout << "OBJ ������ ã�� �� ����." << std::endl;
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
            // ���� ��ġ
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
            vertexCount++;
        }
        else if (prefix == "vt") {
            // �ؽ�ó ��ǥ
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            temp_texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // ���� ����
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            // �� ����
            std::vector<std::string> faceVertices;
            std::string vertexStr;
            while (iss >> vertexStr) {
                faceVertices.push_back(vertexStr);
            }

            if (faceVertices.size() < 3) continue; // �ﰢ���� �ƴ� ���� ����
            faceCount++;

            // "v/vt/vn" ���� �Ľ�
            auto parseVertex = [](const std::string& vertexStr) {
                std::vector<std::string> parts = Split(vertexStr, '/');
                std::vector<unsigned int> indices;
                for (const auto& part : parts) {
                    if (!part.empty()) {
                        indices.push_back(std::stoi(part) - 1); // OBJ�� 1-based �ε���
                    }
                    else {
                        indices.push_back(0); // �� �κ��� 0���� ó��
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
                // �簢���� �� ���� �ﰢ������ ����
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

    //std::cout << "OBJ ���� �Ľ� �Ϸ�: ���� " << vertexCount << "��, �� " << faceCount << "��" << std::endl;

    // �ε����� ����ؼ� ���� ���� �迭 ����
    vertices.clear();
    indices.clear();

    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        Vertex vertex;

        // ��ġ
        if (vertexIndices[i] < temp_vertices.size()) {
            vertex.position = temp_vertices[vertexIndices[i]];
        }

        // �⺻ �Ķ��� �÷� ���� (Tino ĳ���� ����)
        vertex.color = glm::vec3(0.3f, 0.7f, 1.0f);

        // �ؽ�ó ��ǥ
        if (i < texCoordIndices.size() && texCoordIndices[i] < temp_texCoords.size()) {
            vertex.texCoord = temp_texCoords[texCoordIndices[i]];
        }
        else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        // ����
        if (i < normalIndices.size() && normalIndices[i] < temp_normals.size()) {
            vertex.normal = temp_normals[normalIndices[i]];
        }
        else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        vertices.push_back(vertex);
        indices.push_back(static_cast<unsigned int>(i));
    }

    //std::cout << "���� Loaded OBJ: " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;

    if (vertices.empty()) {
        std::cerr << "���: ���� �����Ͱ� �����ϴ�!" << std::endl;
        return false;
    }

    return true;
}

bool Obstacle::LoadTexture(const std::string& texturePath)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // �ؽ�ó �Ű����� ����
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // �̹��� �ε� (���⼭�� �ܻ� �̹��� ���)
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
    std::cout << "obstacle SetupMesh ����" << std::endl;

    // VAO ����
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO ����
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // EBO ����
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // ���� �Ӽ� ����
    // ��ġ (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // �÷� (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // �ؽ�ó ��ǥ (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // 법선 벡터 (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);

    std::cout << "obstacle SetupMesh �Ϸ�, VAO: " << VAO << std::endl;
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
    // ������ ���� ���� �ӵ� ����
    moveSpeed = GetObstacleSpeed();

    // x������ �̵� (�� 60FPS ���� deltaTime = 0.016f ����)
    const float deltaTime = 0.016f;
    position.x += moveSpeed * deltaTime;

    // ����� ��� (��������)
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 60 == 0) { // 1�ʸ��� ���
        //std::cout << "��ֹ� ��ġ: x = " << position.x << ", �ӵ�: " << moveSpeed << std::endl;
    }
}

inline void Obstacle::OnCollision(GameObject* other) {
}


void Obstacle::SetupBoundaryMesh()
{
    // �ڽ��� 8�� ���� ���� (boundary�� r1~r6�� �̿��ؼ� ��ü 8�� ���� ����)
    float boundaryVertices[] = {
        // �޸� 4�� ���� (z = -0.5)
        boundary.r1.x, boundary.r1.y, boundary.r1.z, 0.0f, 1.0f, 0.0f,  // 0: ���� �Ʒ� �� (�ʷϻ�)
        boundary.r2.x, boundary.r2.y, boundary.r2.z, 0.0f, 1.0f, 0.0f,  // 1: ������ �Ʒ� ��
        boundary.r3.x, boundary.r3.y, boundary.r3.z, 0.0f, 1.0f, 0.0f,  // 2: ������ �� ��
        boundary.r4.x, boundary.r4.y, boundary.r4.z, 0.0f, 1.0f, 0.0f,  // 3: ���� �� ��

        // �ո� 4�� ���� (z = +0.5)
        boundary.r5.x, boundary.r5.y, boundary.r5.z, 0.0f, 1.0f, 0.0f,  // 4: ���� �Ʒ� ��
        boundary.r6.x, boundary.r5.y, boundary.r5.z, 0.0f, 1.0f, 0.0f,  // 5: ������ �Ʒ� ��
        boundary.r6.x, boundary.r6.y, boundary.r6.z, 0.0f, 1.0f, 0.0f,  // 6: ������ �� ��
        boundary.r5.x, boundary.r6.y, boundary.r6.z, 0.0f, 1.0f, 0.0f   // 7: ���� �� ��
    };

    glGenVertexArrays(1, &boundaryVAO);
    glGenBuffers(1, &boundaryVBO);

    glBindVertexArray(boundaryVAO);

    glBindBuffer(GL_ARRAY_BUFFER, boundaryVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundaryVertices), boundaryVertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ�
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ���� �Ӽ�
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Obstacle::DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (boundaryVAO == 0) return;

    // �ؽ�ó ��� ����
    glUniform1i(uUseTexture_loc, 0);

    glBindVertexArray(boundaryVAO);

    // ���� ��ֹ��� ��ȯ �� matrix ����
    glm::mat4 model = GetModelMatrix();

    // ��ֹ� Ÿ�Կ� ���� �߰� ��ȯ ����
    if (GetType() == ObstacleType::CACTUS) {
        glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = model * rotate;
    }

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // �� �β� ����
    glLineWidth(2.0f);

    // �ڽ��� 12�� �𼭸��� ������ �׸���
    unsigned int boundaryIndices[] = {
        // �޸��� 4�� �𼭸�
        0, 1,  1, 2,  2, 3,  3, 0,
        // �ո��� 4�� �𼭸�  
        4, 5,  5, 6,  6, 7,  7, 4,
        // �յڸ� �����ϴ� 4�� �𼭸�
        0, 4,  1, 5,  2, 6,  3, 7
    };

    // �ε����� �̿��� �� �׸���
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boundaryIndices);

    // �� �β� ����
    glLineWidth(1.0f);

    glBindVertexArray(0);
}


// Cactus ����
Cactus::Cactus() : Obstacle()
{
    // Cactus ���� boundary ���� (�� ũ�� ����)
    boundary.r1 = glm::vec3(-2.0f, 0.0f, -2.0f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(2.0f, 0.0f, -2.0f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(2.0f, 15.0f, -2.0f);   // ������ �� ��
    boundary.r4 = glm::vec3(-2.0f, 15.0f, -2.0f);  // ���� �� ��
    boundary.r5 = glm::vec3(-2.0f, 0.0f, 2.0f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(2.0f, 15.0f, 2.0f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
    SetupBoundaryMesh();
}

Cactus::Cactus(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{

    scale = glm::vec3(0.2f, 0.2f, 0.2f);

    // Cactus ���� boundary ���� (�� ũ�� ����)
    boundary.r1 = glm::vec3(-2.0f, 0.0f, -2.0f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(2.0f, 0.0f, -2.0f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(2.0f, 15.0f, -2.0f);   // ������ �� ��
    boundary.r4 = glm::vec3(-2.0f, 15.0f, -2.0f);  // ���� �� ��
    boundary.r5 = glm::vec3(-2.0f, 0.0f, 2.0f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(2.0f, 15.0f, 2.0f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
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

    // 변환 행렬들을 셰이더에 전송
    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 조명 계산용 행렬들 전송
    if (uModel_loc >= 0) {
        glUniformMatrix4fv(uModel_loc, 1, GL_FALSE, glm::value_ptr(model));
    }
    if (uView_loc >= 0) {
        glUniformMatrix4fv(uView_loc, 1, GL_FALSE, glm::value_ptr(gView));
    }
    if (uProjection_loc >= 0) {
        glUniformMatrix4fv(uProjection_loc, 1, GL_FALSE, glm::value_ptr(gProjection));
    }

    // Material 설정 제거 - 공통 조명 사용

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);

    // 충돌 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Cactus::Update()
{
    Obstacle::Update(); // �⺻ �̵�

    const float deltaTime = 0.016f;
}


// Tree ����
Tree::Tree() : Obstacle()
{

    // Tree ���� boundary ���� (���� ��翡 �°�)
    boundary.r1 = glm::vec3(-3.0f, 8.0f, -3.0f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(3.0f, 8.0f, -3.0f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(3.0f, 16.0f, -3.0f);   // ������ �� �� (�� ����)
    boundary.r4 = glm::vec3(-3.0f, 16.0f, -3.0f);  // ���� �� ��
    boundary.r5 = glm::vec3(-3.0f, 8.0f, 3.0f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(3.0f, 16.0f, 3.0f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
    SetupBoundaryMesh();
}

Tree::Tree(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{

    scale = glm::vec3(0.2f, 0.2f, 0.2f);

    // Tree ���� boundary ���� (���� ��翡 �°�)
    boundary.r1 = glm::vec3(-3.0f, 8.0f, -3.0f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(3.0f, 8.0f, -3.0f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(3.0f, 16.0f, -3.0f);   // ������ �� �� (�� ����)
    boundary.r4 = glm::vec3(-3.0f, 16.0f, -3.0f);  // ���� �� ��
    boundary.r5 = glm::vec3(-3.0f, 8.0f, 3.0f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(3.0f, 16.0f, 3.0f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
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

    // 변환 행렬들을 셰이더에 전송
    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 조명 계산용 행렬들 전송
    if (uModel_loc >= 0) {
        glUniformMatrix4fv(uModel_loc, 1, GL_FALSE, glm::value_ptr(model));
    }
    if (uView_loc >= 0) {
        glUniformMatrix4fv(uView_loc, 1, GL_FALSE, glm::value_ptr(gView));
    }
    if (uProjection_loc >= 0) {
        glUniformMatrix4fv(uProjection_loc, 1, GL_FALSE, glm::value_ptr(gProjection));
    }

    // Material 설정 제거 - 공통 조명 사용

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);

    // 충돌 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Tree::Update()
{
    Obstacle::Update(); // �⺻ �̵�
}


// Mushroom ����
Mushroom::Mushroom() : Obstacle()
{
    // Mushroom ���� boundary ���� (���� ��翡 �°� - �۰� ����)
    boundary.r1 = glm::vec3(-0.8f, 0.0f, -0.8f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(0.8f, 0.0f, -0.8f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(0.8f, 2.0f, -0.8f);   // ������ �� �� (����)
    boundary.r4 = glm::vec3(-0.8f, 2.0f, -0.8f);  // ���� �� ��
    boundary.r5 = glm::vec3(-0.8f, 0.0f, 0.8f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(0.8f, 2.0f, 0.8f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
    SetupBoundaryMesh();
}

Mushroom::Mushroom(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{
    scale = glm::vec3(1.0f, 1.0f, 1.0f);

    // Mushroom ���� boundary ���� (���� ��翡 �°� - �۰� ����)
    boundary.r1 = glm::vec3(-0.8f, 0.0f, -0.8f); // ���� �Ʒ� ��
    boundary.r2 = glm::vec3(0.8f, 0.0f, -0.8f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(0.8f, 2.0f, -0.8f);   // ������ �� �� (����)
    boundary.r4 = glm::vec3(-0.8f, 2.0f, -0.8f);  // ���� �� ��
    boundary.r5 = glm::vec3(-0.8f, 0.0f, 0.8f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(0.8f, 2.0f, 0.8f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
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

    // 변환 행렬들을 셰이더에 전송
    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 조명 계산용 행렬들 전송
    if (uModel_loc >= 0) {
        glUniformMatrix4fv(uModel_loc, 1, GL_FALSE, glm::value_ptr(model));
    }
    if (uView_loc >= 0) {
        glUniformMatrix4fv(uView_loc, 1, GL_FALSE, glm::value_ptr(gView));
    }
    if (uProjection_loc >= 0) {
        glUniformMatrix4fv(uProjection_loc, 1, GL_FALSE, glm::value_ptr(gProjection));
    }

    // Material 설정 제거 - 공통 조명 사용

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);

    // 충돌 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Mushroom::Update()
{
    Obstacle::Update(); // �⺻ �̵�
}

// Bird ����
Bird::Bird() : Obstacle()
{
    // Bird ���� boundary ���� (�� ��翡 �°� - ���� ����)
    boundary.r1 = glm::vec3(-1.2f, -0.2f, -0.8f); // ���� �Ʒ� �� (���� �� ����)
    boundary.r2 = glm::vec3(1.2f, -0.2f, -0.8f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(1.2f, 1.4f, -0.8f);   // ������ �� �� (�� ����)
    boundary.r4 = glm::vec3(-1.2f, 1.4f, -0.8f);  // ���� �� ��
    boundary.r5 = glm::vec3(-1.2f, -0.2f, 0.8f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(1.2f, 1.4f, 0.8f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
    SetupBoundaryMesh();
}

Bird::Bird(const std::string& objPath, const std::string& texturePath)
    : Obstacle(objPath, texturePath)
{
    position = glm::vec3(SPAWN_X_POSITION, 3.5f, 0.0f);
    scale = glm::vec3(0.9f, 0.9f, 0.9f);
    rotation = glm::vec3(0.0f, -180.0f, 0.0f);

    // Bird ���� boundary ���� (�� ��翡 �°� - ���� ����)
    boundary.r1 = glm::vec3(-1.2f, -0.2f, -0.8f); // ���� �Ʒ� �� (���� �� ����)
    boundary.r2 = glm::vec3(1.2f, -0.2f, -0.8f);  // ������ �Ʒ� ��
    boundary.r3 = glm::vec3(1.2f, 1.4f, -0.8f);   // ������ �� �� (�� ����)
    boundary.r4 = glm::vec3(-1.2f, 1.4f, -0.8f);  // ���� �� ��
    boundary.r5 = glm::vec3(-1.2f, -0.2f, 0.8f);  // ���� �Ʒ� ��
    boundary.r6 = glm::vec3(1.2f, 1.4f, 0.8f);    // ������ �� ��

    // boundary ���� �� boundary �޽� �ٽ� ����
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

    // 변환 행렬들을 셰이더에 전송
    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 조명 계산용 행렬들 전송
    if (uModel_loc >= 0) {
        glUniformMatrix4fv(uModel_loc, 1, GL_FALSE, glm::value_ptr(model));
    }
    if (uView_loc >= 0) {
        glUniformMatrix4fv(uView_loc, 1, GL_FALSE, glm::value_ptr(gView));
    }
    if (uProjection_loc >= 0) {
        glUniformMatrix4fv(uProjection_loc, 1, GL_FALSE, glm::value_ptr(gProjection));
    }

    // Material 설정 제거 - 공통 조명 사용

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);

    // 충돌 박스를 와이어프레임으로 그리기
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Bird::Update()
{
    Obstacle::Update(); // �⺻ �̵�
}


// ObstacleSpawner 구현
ObstacleSpawner::ObstacleSpawner()
    : spawnTimer(0.0f), spawnInterval(2.0f), gen(rd()), dis(0, 3), random_spawnInterval(4.0f, 7.0f)
{
}

void ObstacleSpawner::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
}

void ObstacleSpawner::Update()
{
    if (scene != GameState::PLAYING) {
        spawnTimer = 0.0f;
        return;
    }

    const float deltaTime = 0.016f; // �� 60FPS ����
    spawnTimer += deltaTime;

    // ������ ���� ���� ����
    spawnInterval = random_spawnInterval(gen);

    if (spawnTimer >= spawnInterval) {
        // ���� GameWorld�� �߰����� �ʰ� ��⿭�� �߰�
        SpawnObstacle();
        spawnTimer = 0.0f;
        // ���ο� ���� �������� ����
        spawnInterval = random_spawnInterval(gen);
    }
}

void ObstacleSpawner::SpawnObstacle()
{
    auto obstacle = CreateRandomObstacle();
    if (obstacle) {
        g_gameWorld.AddPendingObject(std::move(obstacle));
    }
}

std::unique_ptr<Obstacle> ObstacleSpawner::CreateRandomObstacle()
{
    int obstacleType = dis(gen);

    switch (obstacleType) {
    case 0:
        std::cout << "������ ����" << std::endl;
        return std::make_unique<Cactus>("assets/obstacle1.obj", "assets/obstacle1_base.bmp");
    case 1:
        std::cout << "���� ����" << std::endl;
        return std::make_unique<Tree>("assets/obstacle2.obj", "assets/obstacle2_base.bmp");
    case 2:
        std::cout << "���� ����" << std::endl;
        return std::make_unique<Mushroom>("assets/obstacle3.obj", "assets/obstacle3_base.bmp");
    case 3:
        std::cout << "�� ����" << std::endl;
        return std::make_unique<Bird>("assets/bird.obj", "assets/bird_base.bmp");
    default:
        // 기본값으로 선인장 생성 (추상 클래스 대신 구체적인 클래스 사용)
        std::cout << "기본 선인장 생성" << std::endl;
        return std::make_unique<Cactus>("assets/obstacle1.obj", "assets/obstacle1_base.bmp");
    }
}