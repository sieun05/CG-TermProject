#include "tino.h"
#include "game_state.h"
#include "LoadBitmap.h"
#include <fstream>
#include <sstream>

// STB 이미지 라이브러리 대신 임시로 텍스처 없이 구현
Tino::Tino(const std::string& objPath, const std::string& jumpPath,
    const std::string& downPath, const std::string& texturePath)
    : textureID(0), bmp(nullptr), state(RUNNING)
{
    // 충돌 영역 설정 (Tino의 기본 크기 기준)
    // r1~r6는 박스의 6개 면을 나타내는 정점들
    boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // 왼쪽 아래 뒤
    boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // 오른쪽 아래 뒤
    boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // 오른쪽 위 뒤
    boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // 왼쪽 위 뒤
    boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // 왼쪽 아래 앞
    boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // 오른쪽 위 앞
    
    LoadOBJ(objPath, RUNNING);
    LoadOBJ(jumpPath, JUMPING);
    LoadOBJ(downPath, SLIDING);

    // 경계 박스 메시 설정
    SetupBoundaryMesh();

	if (LoadTexture(texturePath)) {
		//std::cout << "Tino 텍스처 로딩 성공!" << std::endl;
	}
	else {
		//std::cerr << "Tino 텍스처 로딩 실패!" << std::endl;
	}
}

Tino::~Tino()
{
    for (int i = 0; i < 3; i++) {
        if (meshes[i].VAO != 0) glDeleteVertexArrays(1, &meshes[i].VAO);
        if (meshes[i].VBO != 0) glDeleteBuffers(1, &meshes[i].VBO);
        if (meshes[i].EBO != 0) glDeleteBuffers(1, &meshes[i].EBO);
    }
    if (textureID != 0) glDeleteTextures(1, &textureID);
    
    // 경계 박스 버퍼 정리
    if (boundaryVAO != 0) glDeleteVertexArrays(1, &boundaryVAO);
    if (boundaryVBO != 0) glDeleteBuffers(1, &boundaryVBO);
}

bool Tino::LoadOBJ(const std::string& objPath, State targetState)
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
                    } else {
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
    meshes[targetState].vertices.clear();
    meshes[targetState].indices.clear();

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

        meshes[targetState].vertices.push_back(vertex);
        meshes[targetState].indices.push_back(static_cast<unsigned int>(i));
    }

    //std::cout << "최종 Loaded OBJ: " << meshes[targetState].vertices.size() << " vertices, " << meshes[targetState].indices.size() << " indices" << std::endl;

    if (meshes[targetState].vertices.empty()) {
        std::cerr << "경고: 정점 데이터가 없습니다!" << std::endl;
        return false;
    }

    SetupMesh(targetState);
    meshes[targetState].isLoaded = true;

    return true;
}
bool Tino::LoadTexture(const std::string& texturePath)
{
    // 텍스처 로딩은 나중에 구현
    // std::cout << "Texture loading not implemented yet: " << texturePath << std::endl;
    // 텍스처 로드 및 생성
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 텍스처 매개변수 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 이미지 로드 (여기서는 단색 이미지 사용)
    unsigned char* data = LoadDIBitmap("assets\\Tino_base.bmp", &bmp);
    if (data == NULL) {
        std::cerr << "Failed to load texture: Tino_base" << std::endl;
        return false;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp->bmiHeader.biWidth, bmp->bmiHeader.biHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return true;
}

void Tino::SetupMesh(State targetState)
{
    auto& mesh = meshes[targetState];

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
}

void Tino::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!meshes[state].isLoaded) {
        std::cerr << "Tino가 로드되지 않아서 그릴 수 없습니다" << std::endl;
        return;
    }

    auto& currentMesh = meshes[state];

	glUniform1i(uUseTexture_loc, 1);

    glActiveTexture(GL_TEXTURE0);
	glUniform1i(uTextureSampler_loc, 0);

    glBindVertexArray(currentMesh.VAO);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glm::mat4 model = GetModelMatrix();
	glm::mat4 rotate = glm::mat4(1.0f);
	glm::mat4 translate = glm::mat4(1.0f);
    if (scene == GameState::TITLE) {
        rotate = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else {
        rotate = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translate = glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, 0.0f, 0.0f));
    }
	model = model * rotate;
	model = translate * model;

    glm::mat4 mvp = gProjection * gView * model;

    // 유니폼 설정
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 채워진 삼각형으로 그리기
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(currentMesh.indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // 경계 박스를 와이어프레임으로 그리기 (디버그용)
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Tino::Update()
{
    if (scene == GameState::TITLE) {
        rotation.y += 0.5f;
    }

    // 필요한 업데이트 로직 구현
    if (stateTimer > 0.0f) {
        stateTimer -= 0.016f; // 약 60FPS 가정
        if (state == JUMPING) {
			float top = JUMP_DURATION / 2.0f;
			if (stateTimer > top)
				position += glm::vec3(0.0f, 0.1f, 0.0f); // 점프 시 위로 이동
			else
				position -= glm::vec3(0.0f, 0.1f, 0.0f); // 점프 후 내려오기
			if (position.y < 0.0f)
				position.y = 0.0f;

            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // 왼쪽 아래 뒤
            boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // 오른쪽 아래 뒤
            boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // 오른쪽 위 뒤
            boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // 왼쪽 위 뒤
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // 왼쪽 아래 앞
            boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // 오른쪽 위 앞

            SetupBoundaryMesh();
        }
        if (stateTimer <= 0.0f) {
            state = RUNNING;
            stateTimer = 0.0f;

            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // 왼쪽 아래 뒤
            boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // 오른쪽 아래 뒤
            boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // 오른쪽 위 뒤
            boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // 왼쪽 위 뒤
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // 왼쪽 아래 앞
            boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // 오른쪽 위 앞

            SetupBoundaryMesh();
        }

        if (state == SLIDING) {
            // 슬라이딩 중일 때 경계 박스 조정
            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // 왼쪽 아래 뒤
            boundary.r2 = glm::vec3(0.8f, 0.2f,  -0.8f);  // 오른쪽 아래 뒤
            boundary.r3 = glm::vec3(0.8f, 2.5f,  -0.8f);   // 오른쪽 위 뒤
            boundary.r4 = glm::vec3(-0.8f, 2.5f, -0.8f);  // 왼쪽 위 뒤
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 2.4f);  // 왼쪽 아래 앞
            boundary.r6 = glm::vec3(0.8f, 2.5f,  2.4f);    // 오른쪽 위 앞

            SetupBoundaryMesh();
        }
    }
}

void Tino::StateChange(State newState)
{
	if (state != RUNNING) return; // 현재 RUNNING 상태에서만 상태 변경 허용

    state = newState;

    if (newState == JUMPING) 
		stateTimer = JUMP_DURATION;
	else if (newState == SLIDING)
		stateTimer = SLIDE_DURATION;
    else {
		stateTimer = 0.0f;
    }
}

void Tino::OnCollision(GameObject* other)
{
    // 충돌 시 처리할 로직 구현
    // 예: 장애물과 충돌 시 게임 오버 처리
    std::cout << "Tino가 다른 객체와 충돌했습니다!" << std::endl;
    
    // 필요에 따라 게임 상태 변경이나 다른 처리 로직 추가 가능
    // 예: scene = GameState::GAME_OVER;
}

void Tino::SetupBoundaryMesh()
{
    // 박스의 8개 정점 정의 (boundary의 r1~r6를 이용해서 전체 8개 정점 구성)
    float boundaryVertices[] = {
        // 뒷면 4개 정점 (z = -0.5)
        boundary.r1.x, boundary.r1.y, boundary.r1.z, 1.0f, 0.0f, 0.0f,  // 0: 왼쪽 아래 뒤
        boundary.r2.x, boundary.r2.y, boundary.r2.z, 1.0f, 0.0f, 0.0f,  // 1: 오른쪽 아래 뒤
        boundary.r3.x, boundary.r3.y, boundary.r3.z, 1.0f, 0.0f, 0.0f,  // 2: 오른쪽 위 뒤
        boundary.r4.x, boundary.r4.y, boundary.r4.z, 1.0f, 0.0f, 0.0f,  // 3: 왼쪽 위 뒤
        
        // 앞면 4개 정점 (z = +0.5)
        boundary.r5.x, boundary.r5.y, boundary.r5.z, 1.0f, 0.0f, 0.0f,  // 4: 왼쪽 아래 앞
        boundary.r6.x, boundary.r5.y, boundary.r5.z, 1.0f, 0.0f, 0.0f,  // 5: 오른쪽 아래 앞
        boundary.r6.x, boundary.r6.y, boundary.r6.z, 1.0f, 0.0f, 0.0f,  // 6: 오른쪽 위 앞
        boundary.r5.x, boundary.r6.y, boundary.r6.z, 1.0f, 0.0f, 0.0f   // 7: 왼쪽 위 앞
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

void Tino::DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (boundaryVAO == 0) return;

    // 텍스처 사용 안함
    glUniform1i(uUseTexture_loc, 0);

    glBindVertexArray(boundaryVAO);

    // 현재 Tino의 변환 행렬 적용
    glm::mat4 model = GetModelMatrix();
    glm::mat4 rotate = glm::mat4(1.0f);
    glm::mat4 translate = glm::mat4(1.0f);
    
    if (scene == GameState::TITLE) {
        rotate = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else {
        rotate = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translate = glm::translate(glm::mat4(1.0f), glm::vec3(-7.0f, 0.0f, 0.0f));
    }
    model = model * rotate;
    model = translate * model;

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

    // 인덱스 없이 직접 그리기
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boundaryIndices);
    
    // 선 두께 복원
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
}
