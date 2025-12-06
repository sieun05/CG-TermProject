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
    LoadOBJ(objPath, RUNNING);
    LoadOBJ(jumpPath, JUMPING);
    LoadOBJ(downPath, SLIDING);

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
        rotate = glm::rotate(glm::mat4(1.0f), glm::radians(-15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		rotate = glm::rotate(rotate, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        translate = glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 2.0f, 3.0f));
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
    
    //// 추가로 와이어프레임도 그려서 구조를 확인할 수 있게 함
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glLineWidth(1.0f);
    //glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    //
    //// 다시 FILL 모드로 복원
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
}

void Tino::Update()
{
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
        }
        if (stateTimer <= 0.0f) {
            StateChange(RUNNING);
            stateTimer = 0.0f;
        }
    }
}

void Tino::StateChange(State newState)
{
	if (state == JUMPING || state == SLIDING)
		return; // 점프 또는 슬라이드 중에는 상태 변경 불가

    state = newState;

    if (newState == JUMPING) 
		stateTimer = JUMP_DURATION;
	else if (newState == SLIDING)
		stateTimer = SLIDE_DURATION;
    else {
		stateTimer = 0.0f;
    }
}
