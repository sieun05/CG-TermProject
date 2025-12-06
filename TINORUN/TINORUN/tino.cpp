#include "tino.h"
#include "obstacle.h"
#include "game_state.h"
#include "LoadBitmap.h"
#include <fstream>
#include <sstream>

// STB ?´ë?ì§€ ?¼ì´ë¸ŒëŸ¬ë¦??€???„ì‹œë¡??ìŠ¤ì²??†ì´ êµ¬í˜„
Tino::Tino(const std::string& objPath, const std::string& jumpPath,
    const std::string& downPath, const std::string& texturePath)
    : textureID(0), bmp(nullptr), state(RUNNING)
{
    // ì¶©ëŒ ?ì—­ ?¤ì • (Tino??ê¸°ë³¸ ?¬ê¸° ê¸°ì?)
    // r1~r6??ë°•ìŠ¤??6ê°?ë©´ì„ ?˜í??´ëŠ” ?•ì ??
    boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // ?¼ìª½ ?„ë˜ ??
    boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // ?¤ë¥¸ìª??„ë˜ ??
    boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // ?¤ë¥¸ìª?????
    boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // ?¼ìª½ ????
    boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // ?¼ìª½ ?„ë˜ ??
    boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // ?¤ë¥¸ìª?????
    
    LoadOBJ(objPath, RUNNING);
    LoadOBJ(jumpPath, JUMPING);
    LoadOBJ(downPath, SLIDING);

    // ê²½ê³„ ë°•ìŠ¤ ë©”ì‹œ ?¤ì •
    SetupBoundaryMesh();

	if (LoadTexture(texturePath)) {
		//std::cout << "Tino ?ìŠ¤ì²?ë¡œë”© ?±ê³µ!" << std::endl;
	}
	else {
		//std::cerr << "Tino ?ìŠ¤ì²?ë¡œë”© ?¤íŒ¨!" << std::endl;
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
    
    // ê²½ê³„ ë°•ìŠ¤ ë²„í¼ ?•ë¦¬
    if (boundaryVAO != 0) glDeleteVertexArrays(1, &boundaryVAO);
    if (boundaryVBO != 0) glDeleteBuffers(1, &boundaryVBO);
}

bool Tino::LoadOBJ(const std::string& objPath, State targetState)
{
    //std::cout << "OBJ ?Œì¼ ë¡œë”© ?œì‘: " << objPath << std::endl;
    
    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;
        
        // ?¤ë¥¸ ê²½ë¡œ?¤ë„ ?œë„?´ë³´ê¸?
        std::string altPath1 = objPath;
        std::string altPath2 = ".." + objPath;
        std::string altPath3 = "./TINORUN/" + objPath;
        
        std::cout << "?€ì²?ê²½ë¡œ ?œë„: " << altPath1 << std::endl;
        file.open(altPath1);
        if (!file.is_open()) {
            std::cout << "?€ì²?ê²½ë¡œ ?œë„: " << altPath2 << std::endl;
            file.open(altPath2);
            if (!file.is_open()) {
                std::cout << "?€ì²?ê²½ë¡œ ?œë„: " << altPath3 << std::endl;
                file.open(altPath3);
                if (!file.is_open()) {
                    std::cout << "OBJ ?Œì¼??ì°¾ì„ ???†ìŒ." << std::endl;
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
            // ?•ì  ?„ì¹˜
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
            vertexCount++;
        }
        else if (prefix == "vt") {
            // ?ìŠ¤ì²?ì¢Œí‘œ
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            temp_texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // ë²•ì„  ë²¡í„°
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            // ë©??•ë³´
			std::vector<std::string> faceVertices;
            std::string vertexStr;
			while (iss >> vertexStr) {
				faceVertices.push_back(vertexStr);
			}

			if (faceVertices.size() < 3) continue; // ?¼ê°?•ì´ ?„ë‹Œ ë©´ì? ë¬´ì‹œ
			faceCount++;
            
            // "v/vt/vn" ?•ì‹ ?Œì‹±
            auto parseVertex = [](const std::string& vertexStr) {
                std::vector<std::string> parts = Split(vertexStr, '/');
                std::vector<unsigned int> indices;
                for (const auto& part : parts) {
                    if (!part.empty()) {
                        indices.push_back(std::stoi(part) - 1); // OBJ??1-based ?¸ë±??
                    } else {
                        indices.push_back(0); // ë¹?ë¶€ë¶„ì? 0?¼ë¡œ ì²˜ë¦¬
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
                // ?¬ê°?•ì„ ??ê°œì˜ ?¼ê°?•ìœ¼ë¡?ë¶„í• 
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

    //std::cout << "OBJ ?Œì¼ ?Œì‹± ?„ë£Œ: ?•ì  " << vertexCount << "ê°? ë©?" << faceCount << "ê°? << std::endl;

    // ?¸ë±?¤ë? ?¬ìš©?´ì„œ ìµœì¢… ?•ì  ë°°ì—´ êµ¬ì„±
    meshes[targetState].vertices.clear();
    meshes[targetState].indices.clear();

    for (size_t i = 0; i < vertexIndices.size(); ++i) {
        Vertex vertex;

        // ?„ì¹˜
        if (vertexIndices[i] < temp_vertices.size()) {
            vertex.position = temp_vertices[vertexIndices[i]];
        }

        // ê¸°ë³¸ ?Œë???ì»¬ëŸ¬ ?¤ì • (Tino ìºë¦­???‰ìƒ)
        vertex.color = glm::vec3(0.3f, 0.7f, 1.0f);

        // ?ìŠ¤ì²?ì¢Œí‘œ
        if (i < texCoordIndices.size() && texCoordIndices[i] < temp_texCoords.size()) {
            vertex.texCoord = temp_texCoords[texCoordIndices[i]];
        } else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        // ë²•ì„ 
        if (i < normalIndices.size() && normalIndices[i] < temp_normals.size()) {
            vertex.normal = temp_normals[normalIndices[i]];
        } else {
            vertex.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }

        meshes[targetState].vertices.push_back(vertex);
        meshes[targetState].indices.push_back(static_cast<unsigned int>(i));
    }

    //std::cout << "ìµœì¢… Loaded OBJ: " << meshes[targetState].vertices.size() << " vertices, " << meshes[targetState].indices.size() << " indices" << std::endl;

    if (meshes[targetState].vertices.empty()) {
        std::cerr << "ê²½ê³ : ?•ì  ?°ì´?°ê? ?†ìŠµ?ˆë‹¤!" << std::endl;
        return false;
    }

    SetupMesh(targetState);
    meshes[targetState].isLoaded = true;

    return true;
}
bool Tino::LoadTexture(const std::string& texturePath)
{
    // ?ìŠ¤ì²?ë¡œë”©?€ ?˜ì¤‘??êµ¬í˜„
    // std::cout << "Texture loading not implemented yet: " << texturePath << std::endl;
    // ?ìŠ¤ì²?ë¡œë“œ ë°??ì„±
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // ?ìŠ¤ì²?ë§¤ê°œë³€???¤ì •
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ?´ë?ì§€ ë¡œë“œ (?¬ê¸°?œëŠ” ?¨ìƒ‰ ?´ë?ì§€ ?¬ìš©)
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
        std::cerr << "Tinoê°€ ë¡œë“œ?˜ì? ?Šì•„??ê·¸ë¦´ ???†ìŠµ?ˆë‹¤" << std::endl;
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

    // ? ë‹ˆ???¤ì •
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // ì±„ì›Œì§??¼ê°?•ìœ¼ë¡?ê·¸ë¦¬ê¸?
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(currentMesh.indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(uUseTexture_loc, 0);
    
    // ê²½ê³„ ë°•ìŠ¤ë¥??€?´ì–´?„ë ˆ?„ìœ¼ë¡?ê·¸ë¦¬ê¸?(?”ë²„ê·¸ìš©)
    DrawBoundary(gProjection, gView, uMVP_loc);
}

void Tino::Update()
{
    if (scene == GameState::TITLE) {
        rotation.y += 0.5f;
    }

    // ?„ìš”???…ë°?´íŠ¸ ë¡œì§ êµ¬í˜„
    if (stateTimer > 0.0f) {
        stateTimer -= 0.016f; // ??60FPS ê°€??
        if (state == JUMPING) {
			float top = JUMP_DURATION / 2.0f;
			if (stateTimer > top)
				position += glm::vec3(0.0f, 0.1f, 0.0f); // ?í”„ ???„ë¡œ ?´ë™
			else
				position -= glm::vec3(0.0f, 0.1f, 0.0f); // ?í”„ ???´ë ¤?¤ê¸°
			if (position.y < 0.0f)
				position.y = 0.0f;

            boundary.r1 = glm::vec3(-0.8f, 0.4f, -0.8f); // ?¼ìª½ ?„ë˜ ??
            boundary.r2 = glm::vec3(0.8f, 0.4f, -0.8f);  // ?¤ë¥¸ìª??„ë˜ ??
            boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // ?¤ë¥¸ìª?????
            boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // ?¼ìª½ ????
            boundary.r5 = glm::vec3(-0.8f, 0.4f, 0.8f);  // ?¼ìª½ ?„ë˜ ??
            boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // ?¤ë¥¸ìª?????

            SetupBoundaryMesh();
        }
        if (stateTimer <= 0.0f) {
            state = RUNNING;
            stateTimer = 0.0f;

            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // ?¼ìª½ ?„ë˜ ??
            boundary.r2 = glm::vec3(0.8f, 0.2f, -0.8f);  // ?¤ë¥¸ìª??„ë˜ ??
            boundary.r3 = glm::vec3(0.8f, 3.8f, -0.8f);   // ?¤ë¥¸ìª?????
            boundary.r4 = glm::vec3(-0.8f, 3.8f, -0.8f);  // ?¼ìª½ ????
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 0.8f);  // ?¼ìª½ ?„ë˜ ??
            boundary.r6 = glm::vec3(0.8f, 3.8f, 0.8f);    // ?¤ë¥¸ìª?????

            SetupBoundaryMesh();
        }

        if (state == SLIDING) {
            // ?¬ë¼?´ë”© ì¤‘ì¼ ??ê²½ê³„ ë°•ìŠ¤ ì¡°ì •
            boundary.r1 = glm::vec3(-0.8f, 0.2f, -0.8f); // ?¼ìª½ ?„ë˜ ??
            boundary.r2 = glm::vec3(0.8f, 0.2f,  -0.8f);  // ?¤ë¥¸ìª??„ë˜ ??
            boundary.r3 = glm::vec3(0.8f, 2.5f,  -0.8f);   // ?¤ë¥¸ìª?????
            boundary.r4 = glm::vec3(-0.8f, 2.5f, -0.8f);  // ?¼ìª½ ????
            boundary.r5 = glm::vec3(-0.8f, 0.2f, 2.4f);  // ?¼ìª½ ?„ë˜ ??
            boundary.r6 = glm::vec3(0.8f, 2.5f,  2.4f);    // ?¤ë¥¸ìª?????

            SetupBoundaryMesh();
        }
    }
}

void Tino::StateChange(State newState)
{
	if (state != RUNNING) return; // ?„ì¬ RUNNING ?íƒœ?ì„œë§??íƒœ ë³€ê²??ˆìš©

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
    // ì¶©ëŒ ??ì²˜ë¦¬??ë¡œì§ êµ¬í˜„
    // ?? ?¥ì• ë¬¼ê³¼ ì¶©ëŒ ??ê²Œì„ ?¤ë²„ ì²˜ë¦¬
    std::cout << "Tino collision detected!" << std::endl;
    
    // ?„ìš”???°ë¼ ê²Œì„ ?íƒœ ë³€ê²½ì´???¤ë¥¸ ì²˜ë¦¬ ë¡œì§ ì¶”ê? ê°€??
    // ?? scene = GameState::GAME_OVER;
}

void Tino::SetupBoundaryMesh()
{
    // ë°•ìŠ¤??8ê°??•ì  ?•ì˜ (boundary??r1~r6ë¥??´ìš©?´ì„œ ?„ì²´ 8ê°??•ì  êµ¬ì„±)
    float boundaryVertices[] = {
        // ?·ë©´ 4ê°??•ì  (z = -0.5)
        boundary.r1.x, boundary.r1.y, boundary.r1.z, 1.0f, 0.0f, 0.0f,  // 0: ?¼ìª½ ?„ë˜ ??
        boundary.r2.x, boundary.r2.y, boundary.r2.z, 1.0f, 0.0f, 0.0f,  // 1: ?¤ë¥¸ìª??„ë˜ ??
        boundary.r3.x, boundary.r3.y, boundary.r3.z, 1.0f, 0.0f, 0.0f,  // 2: ?¤ë¥¸ìª?????
        boundary.r4.x, boundary.r4.y, boundary.r4.z, 1.0f, 0.0f, 0.0f,  // 3: ?¼ìª½ ????
        
        // ?ë©´ 4ê°??•ì  (z = +0.5)
        boundary.r5.x, boundary.r5.y, boundary.r5.z, 1.0f, 0.0f, 0.0f,  // 4: ?¼ìª½ ?„ë˜ ??
        boundary.r6.x, boundary.r5.y, boundary.r5.z, 1.0f, 0.0f, 0.0f,  // 5: ?¤ë¥¸ìª??„ë˜ ??
        boundary.r6.x, boundary.r6.y, boundary.r6.z, 1.0f, 0.0f, 0.0f,  // 6: ?¤ë¥¸ìª?????
        boundary.r5.x, boundary.r6.y, boundary.r6.z, 1.0f, 0.0f, 0.0f   // 7: ?¼ìª½ ????
    };

    glGenVertexArrays(1, &boundaryVAO);
    glGenBuffers(1, &boundaryVBO);

    glBindVertexArray(boundaryVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, boundaryVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundaryVertices), boundaryVertices, GL_STATIC_DRAW);

    // ?„ì¹˜ ?ì„±
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ?‰ìƒ ?ì„±
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Tino::DrawBoundary(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (boundaryVAO == 0) return;

    // ?ìŠ¤ì²??¬ìš© ?ˆí•¨
    glUniform1i(uUseTexture_loc, 0);

    glBindVertexArray(boundaryVAO);

    // ?„ì¬ Tino??ë³€???‰ë ¬ ?ìš©
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

    // ???ê»˜ ?¤ì •
    glLineWidth(2.0f);
    
    // ë°•ìŠ¤??12ê°?ëª¨ì„œë¦¬ë? ? ìœ¼ë¡?ê·¸ë¦¬ê¸?
    unsigned int boundaryIndices[] = {
        // ?·ë©´??4ê°?ëª¨ì„œë¦?
        0, 1,  1, 2,  2, 3,  3, 0,
        // ?ë©´??4ê°?ëª¨ì„œë¦? 
        4, 5,  5, 6,  6, 7,  7, 4,
        // ?ë’¤ë¥??°ê²°?˜ëŠ” 4ê°?ëª¨ì„œë¦?
        0, 4,  1, 5,  2, 6,  3, 7
    };

    // ?¸ë±???†ì´ ì§ì ‘ ê·¸ë¦¬ê¸?
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, boundaryIndices);
    
    // ???ê»˜ ë³µì›
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
}
