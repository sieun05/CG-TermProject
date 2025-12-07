#include "tino.h"
#include "obstacle.h"
#include "game_state.h"
#include "LoadBitmap.h"
#include <fstream>
#include <sstream>

// STB ?대?吏 ?쇱씠釉뚮윭由?????꾩떆濡??띿뒪泥??놁씠 揦ы睛
Tino::Tino(const std::string& objPath, const std::string& jumpPath,
    const std::string& downPath, const std::string& texturePath)
    : textureID(0), bmp(nullptr), state(RUNNING)
{
    // 異⑸룎 ?곸뿭 ?ㅼ젙 (Tino??湲곕낯 ?ш린湲곗?)
    // r1~r6??諛뺤뒪??6媛?硫댁쓣 ?섑??대뒗 ?뺤젏??
    boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // ?쇱そ ?꾨옒 ??
    boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // ?ㅻⅨ履??꾨옒 ??
    boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // ?ㅻⅨ履?????
    boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // ?쇱そ ????
    boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // ?쇱そ ?꾨옒 ??
    boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // ?ㅻⅨ履?????
    
    LoadOBJ(objPath, RUNNING);
    LoadOBJ(jumpPath, JUMPING);
    LoadOBJ(downPath, SLIDING);

    // 寃쎄퀎 諛뺤뒪 硫붿떆 ?ㅼ젙
    SetupBoundaryMesh();

	if (LoadTexture(texturePath)) {
		//std::cout << "Tino ?띿뒪泥?濡쒕뵫 ?깃났!" << std::endl;
	}
	else {
		//std::cerr << "Tino ?띿뒪泥?濡쒕뵫 ?ㅽ뙣!" << std::endl;
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
    
    // 寃쎄퀎 諛뺤뒪 踰꾪띁 ?뺣━
    if (boundaryVAO != 0) glDeleteVertexArrays(1, &boundaryVAO);
    if (boundaryVBO != 0) glDeleteBuffers(1, &boundaryVBO);
}

bool Tino::LoadOBJ(const std::string& objPath, State targetState)
{
    //std::cout << "OBJ ?뚯씪 濡쒕뵫 ?쒖옉: " << objPath << std::endl;
    
    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;
        
        // ?ㅻⅨ 寃쎈줈?ㅻ룄 ?쒕룄?대낫湲?
        std::string altPath1 = objPath;
        std::string altPath2 = ".." + objPath;
        std::string altPath3 = "./TINORUN/" + objPath;
        
        std::cout << "?泥?寃쎈줈 ?쒕룄: " << altPath1 << std::endl;
        file.open(altPath1);
        if (!file.is_open()) {
            std::cout << "?泥?寃쎈줈 ?쒕룄: " << altPath2 << std::endl;
            file.open(altPath2);
            if (!file.is_open()) {
                std::cout << "?泥?寃쎈줈 ?쒕룄: " << altPath3 << std::endl;
                file.open(altPath3);
                if (!file.is_open()) {
                    std::cout << "OBJ ?뚯씪??李얠쓣 ???놁쓬." << std::endl;
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
            // ?뺤젏 ?꾩튂
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
            vertexCount++;
        }
        else if (prefix == "vt") {
            // ?띿뒪泥?醫뚰몴
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            temp_texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // 踰뺤꽑 踰≫꽣
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            // 硫??뺣낫
			std::vector<std::string> faceVertices;
            std::string vertexStr;
			while (iss >> vertexStr) {
				faceVertices.push_back(vertexStr);
			}

			if (faceVertices.size() < 3) continue; // ?쇨컖?뺤씠 ?꾨땶 硫댁? 臾댁떆
			faceCount++;
            
            // "v/vt/vn" ?뺤떇 ?뚯떛
            auto parseVertex = [](const std::string& vertexStr) {
                std::vector<std::string> parts = Split(vertexStr, '/');
                std::vector<unsigned int> indices;
                for (const auto& part : parts) {
                    if (!part.empty()) {
                        indices.push_back(std::stoi(part) - 1); // OBJ??1-based ?몃뜳??
                    } else {
                        indices.push_back(0); // 鍮?遺遺꾩? 0?쇰줈 泥섎━
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
                // ?ш컖?뺤쓣 ??媛쒖쓽 ?쇨컖?뺤쑝濡?遺꾪븷
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

    //std::cout << "OBJ ?뚯씪 ?뚯떛 ?꾨즺: ?뺤젏 " << vertexCount << "媛? 硫?" << faceCount << "媛? << std::endl;

    // ?몃뜳?ㅻ? ?ъ슜?댁꽌 理쒖쥌 ?뺤젏 諛곗뿴 揦ъ꽦
    meshes[targetState].vertices.clear();
    meshes[targetState].indices.clear();

    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        Vertex vertex;

        // ?꾩튂
        if (vertexIndices[i] < temp_vertices.size()) {
            vertex.position = temp_vertices[vertexIndices[i]];
        }

        // 湲곕낯 ?뚮???而щ윭 ?ㅼ젙 (Tino 罹먮┃???됱긽)
        vertex.color = glm::vec3(0.3f, 0.7f, 1.0f);

        // ?띿뒪泥?醫뚰몴
        if (i < texCoordIndices.size() && texCoordIndices[i] < temp_texCoords.size()) {
            vertex.texCoord = temp_texCoords[texCoordIndices[i]];
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        // 踰뺤꽑
        if (i < normalIndices.size() && normalIndices[i] < temp_normals.size()) {
            vertex.normal = temp_normals[normalIndices[i]];
        } else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        meshes[targetState].vertices.push_back(vertex);
        meshes[targetState].indices.push_back(static_cast<unsigned int>(i));
    }

    //std::cout << "理쒖쥌 Loaded OBJ: " << meshes[targetState].vertices.size() << " vertices, " << meshes[targetState].indices.size() << " indices" << std::endl;

    if (meshes[targetState].vertices.empty()) {
        std::cerr << "寃쎄퀬: ?뺤젏 ?곗씠?곌? ?놁뒿?덈떎!" << std::endl;
        return false;
    }

    SetupMesh(targetState);
    meshes[targetState].isLoaded = true;

    return true;
}
bool Tino::LoadTexture(const std::string& texturePath)
{
    // ?띿뒪泥?濡쒕뵫? ?섏쨷??援ы쁽
    // std::cout << "Texture loading not implemented yet: " << texturePath << std::endl;
    // ?띿뒪泥?濡쒕뱶 諛??앹꽦
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // ?띿뒪泥?留ㅺ컻蹂???ㅼ젙
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ?대?吏 濡쒕뱶 (?ш린?쒕뒗 ?⑥깋 ?대?吏 ?ъ슜)
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

    // 위치 (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // 컬러 (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // 텍스처 좌표 (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // 법선 벡터 (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
}

void Tino::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (!meshes[state].isLoaded) {
        std::cerr << "Tino가 로드되지 않아 그릴 수 없습니다" << std::endl;
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
    
    // 재질 설정 - Tino용 (플라스틱 계열의 밝고 부드러운 재질)
    Material tinoMaterial = Material(
        glm::vec3(0.4f, 0.375f, 0.35f),        // 파란색 계열 환경광
        glm::vec3(0.8f, 0.6f, 0.2f),        // 밝은 파란색 확산광
        glm::vec3(0.5f, 0.8f, 1.0f),        // 밝고 부드러운 반사광
        64.0f                                // 높은 광택도 (부드러운 표면)
    );
    g_lightManager.SendMaterialToShader(tinoMaterial);

    // 채워진 삼각형으로 그리기
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(currentMesh.indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // 경계 박스를 와이어프레임으로 그리기(디버깅용)
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Tino::Update()
{
    if (scene == GameState::TITLE) {
        rotation.y += 0.5f;
    }

    // ?꾩슂???낅뜲?댄듃 濡쒖쭅 揦и쁽
    if (stateTimer > 0.0f) {
        stateTimer -= 0.016f; // ??60FPS 媛??
        if (state == JUMPING) {
			float top = JUMP_DURATION / 2.0f;
			if (stateTimer > top)
				position += glm::vec3(0.0f, 0.1f, 0.0f); // ?먰봽 ???꾨줈 ?대룞
			else
				position -= glm::vec3(0.0f, 0.1f, 0.0f); // ?먰봽 ???대젮?ㅺ린
			if (position.y < 0.5f)
				position.y = 0.5f;

            boundary.r1 = glm::vec3(-0.8f, 0.4f, -0.8f); // ?쇱そ ?꾨옒 ??
            boundary.r2 = glm::vec3(0.8f, 0.4f, -0.8f);  // ?ㅻⅨ履??꾨옒 ??
            boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // ?ㅻⅨ履?????
            boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // ?쇱そ ????
            boundary.r5 = glm::vec3(-0.8f, 0.4f, 0.8f);  // ?쇱そ ?꾨옒 ??
            boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // ?ㅻⅨ履?????

            SetupBoundaryMesh();
        }
        if (stateTimer <= 0.0f) {
            state = RUNNING;
            stateTimer = 0.0f;

            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // ?쇱そ ?꾨옒 ??
            boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // ?ㅻⅨ履??꾨옒 ??
            boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // ?ㅻⅨ履?????
            boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // ?쇱そ ????
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // ?쇱そ ?꾨옒 ??
            boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // ?ㅻⅨ履?????

            SetupBoundaryMesh();
        }

        if (state == SLIDING) {
            // ?щ씪?대뵫 以묒씪 ??寃쎄퀎 諛뺤뒪 議곗젙
            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // ?쇱そ ?꾨옒 ??
            boundary.r2 = glm::vec3(0.8f, 0.2f,  -0.8f);  // ?ㅻⅨ履??꾨옒 ??
            boundary.r3 = glm::vec3(0.8f, 2.5f,  -0.8f);   // ?ㅻⅨ履?????
            boundary.r4 = glm::vec3(-0.8f, 2.5f, -0.8f);  // ?쇱そ ????
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 2.4f);  // ?쇱そ ?꾨옒 ??
            boundary.r6 = glm::vec3(0.8f, 2.5f,  2.4f);    // ?ㅻⅨ履?????

            SetupBoundaryMesh();
        }
    }
}

void Tino::StateChange(State newState)
{
	if (state != RUNNING) return; // ?꾩옱 RUNNING ?곹깭?먯꽌留??곹깭 蹂寃??덉슜

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
    // Check if the other object is an obstacle
    Obstacle* obstacle = dynamic_cast<Obstacle*>(other);
    if (obstacle) {
        // Set scene to GAME_OVER when collision with obstacle occurs
        //scene = GameState::GAME_OVER;
        // Initialize game over screen immediately
        gameover_flag222 = true;
    }
}

void Tino::SetupBoundaryMesh()
{
    // 諛뺤뒪??8媛??뺤젏 ?뺤쓽 (boundary??r1~r6瑜??댁슜?댁꽌 ?꾩껜 8媛??뺤젏 揦ъ꽦)
    float boundaryVertices[] = {
        // ?룸㈃ 4媛??뺤젏 (z = -0.5)
        boundary.r1.x, boundary.r1.y, boundary.r1.z, 1.0f, 0.0f, 0.0f,  // 0: ?쇱そ ?꾨옒 ??
        boundary.r2.x, boundary.r2.y, boundary.r2.z, 1.0f, 0.0f, 0.0f,  // 1: ?ㅻⅨ履??꾨옒 ??
        boundary.r3.x, boundary.r3.y, boundary.r3.z, 1.0f, 0.0f, 0.0f,  // 2: ?ㅻⅨ履?????
        boundary.r4.x, boundary.r4.y, boundary.r4.z, 1.0f, 0.0f, 0.0f,  // 3: ?쇱そ ????
        
        // ?욌㈃ 4媛??뺤젏 (z = +0.5)
        boundary.r5.x, boundary.r5.y, boundary.r5.z, 1.0f, 0.0f, 0.0f,  // 4: ?쇱そ ?꾨옒 ??
        boundary.r6.x, boundary.r5.y, boundary.r5.z, 1.0f, 0.0f, 0.0f,  // 5: ?ㅻⅨ履??꾨옒 ??
        boundary.r6.x, boundary.r6.y, boundary.r6.z, 1.0f, 0.0f, 0.0f,  // 6: ?ㅻⅨ履?????
        boundary.r5.x, boundary.r6.y, boundary.r6.z, 1.0f, 0.0f, 0.0f   // 7: ?쇱そ ????
    };

    glGenVertexArrays(1, &boundaryVAO);
    glGenBuffers(1, &boundaryVBO);

    glBindVertexArray(boundaryVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, boundaryVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundaryVertices), boundaryVertices, GL_STATIC_DRAW);

    // ?꾩튂 ?띿꽦
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ?됱긽 ?띿꽦
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Tino::DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (boundaryVAO == 0) return;

    // ?띿뒪泥??ъ슜 ?덊븿
    glUniform1i(uUseTexture_loc, 0);

    glBindVertexArray(boundaryVAO);

    // ?꾩옱 Tino??蹂???됰젹 ?곸슜
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

    // ???먭퍡 ?ㅼ젙
    glLineWidth(2.0f);
    
    // 諛뺤뒪??12媛?紐⑥꽌由щ? ?좎쑝濡?洹몃━湲?
    unsigned int boundaryIndices[] = {
        // ?룸㈃??4媛?紐⑥꽌由?
        0, 1,  1, 2,  2, 3,  3, 0,
        // ?욌㈃??4媛?紐⑥꽌由? 
        4, 5,  5, 6,  6, 7,  7, 4,
        // ?욌뮘瑜??곌껐?섎뒗 4媛?紐⑥꽌由?
        0, 4,  1, 5,  2, 6,  3, 7
    };

    // ?몃뜳???놁씠 吏곸젒 洹몃━湲?
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boundaryIndices);
    
    // ???먭퍡 蹂듭썝
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
}
