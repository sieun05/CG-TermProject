#include "Button.h"
#include "game_state.h"
#include "LoadBitmap.h"

Button::Button(float x, float y, float w, float h)
	: x(x), y(y), w(w), h(h)
{
	InitBuffer();

	position = glm::vec3(x, y, 0.0f);
	scale = glm::vec3(w, h, 1.0f);
}

void Button::InitBuffer()
{
	float vertices[] = {
		// 위치         // 텍스처 좌표
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // 왼쪽 아래
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // 오른쪽 아래
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // 오른쪽 위
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f  // 왼쪽 위
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
	// 위치 속성
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// 텍스처 좌표 속성
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void Button::InitTexture()
{
	// 텍스처 로드 및 생성
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// 텍스처 매개변수 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 이미지 로드 (여기서는 단색 이미지 사용)
	unsigned char* data = LoadDIBitmap("assets\start_button.bmp", &bmp);
	if (data == NULL) {
		std::cerr << "Failed to load texture: 시작 버튼" << std::endl;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 500, 212, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
}

void Button::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
	// 시작 화면이 아니라면 그리지 않음
	if (scene != GameState::TITLE) return;
	
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, scale);
	model = glm::translate(model, position);
	glm::mat4 mvp = glm::mat4(1.0f) * model;

	glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);
	glUniform1d(uUseTexture_loc, true); // 텍스처 사용
	
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Button::Update()
{
}
