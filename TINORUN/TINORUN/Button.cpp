#include "Button.h"
#include "game_state.h"
#include "LoadBitmap.h"

Button::Button(float x, float y, float w, float h, const std::string& texturePath)
	: x(x), y(y), w(w), h(h)
{
	InitBuffer();
	LoadTexture(texturePath);

	position = glm::vec3(x, y, 0.0f);
	scale = glm::vec3(w, h, 1.0f);
}

void Button::InitBuffer()
{
	float vertices[] = {
		// 위치                // 컬러              // 텍스처 좌표
		-0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // 왼쪽 아래
		 0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 오른쪽 아래
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 오른쪽 위
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // 왼쪽 위
	};
	unsigned int indices[] = {
		0, 1, 2, // 첫 번째 삼각형
		2, 3, 0  // 두 번째 삼각형
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 위치 속성 (location 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// 컬러 속성 (location 1)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// 텍스처 좌표 속성 (location 2)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

bool Button::LoadTexture(const std::string& texturePath)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// 텍스처 매개변수 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 이미지 로드 (여기서는 단색 이미지 사용)
	/*int width, height, channels;
	unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);*/
	unsigned char* data = LoadDIBitmap(texturePath.c_str(), &bmp);
	if (data == NULL) {
		std::cerr << "Failed to load texture: " << texturePath << std::endl;
		return false;
	}

	/*GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);*/

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp->bmiHeader.biWidth, bmp->bmiHeader.biHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	return true;
}

void Button::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{	
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, scale);
	model = glm::translate(model, position);
	glm::mat4 mvp = glm::mat4(1.0f) * model;

	glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);
	glUniform1i(uUseTexture_loc, 1); // 텍스처 사용

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(uTextureSampler_loc, 0); // 텍스처 유닛 0 사용
	
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(uUseTexture_loc, 0); // 텍스처 사용 안함
}

void Button::Update()
{
	// 버튼은 특별한 업데이트가 필요 없음
	return;
}
