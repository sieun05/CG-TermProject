#include "ScoreDisplay.h"
#include "game_state.h"
#include "LoadBitmap.h"
#include <algorithm>

extern "C" void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

ScoreDisplay::ScoreDisplay()
	: currentScore(0), digitWidth(0.3f), digitHeight(0.4f), spacing(0.01f)
{
}

ScoreDisplay::ScoreDisplay(float x, float y, float digitWidth, float digitHeight, const std::string& texturePath)
	: currentScore(0), digitWidth(digitWidth), digitHeight(digitHeight), spacing(0.01f)
{
	position = glm::vec3(x, y, 0.0f);
	InitBuffer();
	LoadTexture(texturePath);
}

ScoreDisplay::~ScoreDisplay()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &textureID);
}

void ScoreDisplay::InitBuffer()
{
	// 한 숫자의 정점 데이터 (텍스처 좌표는 DrawDigit에서 동적으로 설정)
	float vertices[] = {
		// 위치                // 컬러              // 텍스처 좌표
		-0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // 왼쪽 아래
		 0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.1f, 0.0f, // 오른쪽 아래
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.1f, 1.0f, // 오른쪽 위
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); // DYNAMIC_DRAW로 변경

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

bool ScoreDisplay::LoadTexture(const std::string& texturePath)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// 텍스처 매개변수 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 이미지 뒤집기 (OpenGL 좌표계에 맞춤)
	stbi_set_flip_vertically_on_load(true);

	// 이미지 로드
	int width, height, channels;
	unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
	if (data == NULL) {
		std::cerr << "Failed to load score texture: " << texturePath << std::endl;
		return false;
	}

	GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	return true;
}

void ScoreDisplay::SetScore(int score)
{
	// 점수를 0-99999 범위로 제한
	currentScore = std::max(0, std::min(99999, score));
}

float ScoreDisplay::DrawDigit(int digit, float x, float y, glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
	// digit은 0-9 사이의 값
	digit = std::max(0, std::min(9, digit));

	// 스프라이트 시트의 배열: 1,2,3,4,5,6,7,8,9,0
	// 각 숫자의 텍스처 좌표 및 상대적 너비
	struct DigitCoord {
		float left;
		float right;
		float widthRatio;  // 기본 너비 대비 비율
	};

	// 숫자 배열 순서: 1,2,3,4,5,6,7,8,9,0
	DigitCoord coords[10] = {
		{0.90f, 1.00f, 1.0f},  // 0 - 맨 오른쪽
		{0.00f, 0.10f, 1.0f},  // 1 - 맨 왼쪽, 매우 좁음
		{0.10f, 0.20f, 1.0f},  // 2
		{0.20f, 0.30f, 1.0f},  // 3
		{0.30f, 0.40f, 1.0f},  // 4
		{0.40f, 0.50f, 1.0f},  // 5
		{0.50f, 0.60f, 1.0f},  // 6
		{0.60f, 0.70f, 1.0f},  // 7
		{0.70f, 0.80f, 1.0f},  // 8
		{0.80f, 0.90f, 1.0f}   // 9
	};

	float texLeft = coords[digit].left;
	float texRight = coords[digit].right;
	float actualWidth = digitWidth * coords[digit].widthRatio;  // 실제 렌더링 너비

	// 버퍼 업데이트 (텍스처 좌표만 변경)
	float vertices[] = {
		// 위치                // 컬러              // 텍스처 좌표
		-0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   texLeft,  0.0f, // 왼쪽 아래
		 0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   texRight, 0.0f, // 오른쪽 아래
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   texRight, 1.0f, // 오른쪽 위
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   texLeft,  1.0f  // 왼쪽 위
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	// 모델 변환 행렬 계산 (실제 너비 적용)
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(x, y, 0.0f));
	model = glm::scale(model, glm::vec3(actualWidth, digitHeight, 1.0f));
	glm::mat4 mvp = glm::mat4(1.0f) * model;

	glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// 실제 사용된 너비 반환
	return actualWidth;
}

void ScoreDisplay::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
	glUniform1i(uUseTexture_loc, 1); // 텍스처 사용

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(uTextureSampler_loc, 0); // 텍스처 유닛 0 사용

	// 알파 블렌딩 활성화 (투명 배경 지원)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 5자리 숫자로 분리
	int digits[5];
	int temp = currentScore;
	for (int i = 4; i >= 0; i--) {
		digits[i] = temp % 10;
		temp /= 10;
	}

	// 각 자리수를 그림 (왼쪽에서 오른쪽으로)
	float startX = position.x;
	float currentX = startX;


	for (int i = 0; i < 5; i++) {
		// 각 숫자를 그리고 실제 사용된 너비를 받아옴
		float usedWidth = DrawDigit(digits[i], currentX, position.y, gProjection, gView, uMVP_loc);
		// 다음 숫자 위치 = 현재 위치 + 실제 너비 + 간격
		currentX += usedWidth + spacing;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND); // 블렌딩 비활성화
	glUniform1i(uUseTexture_loc, 0); // 텍스처 사용 안함

}

void ScoreDisplay::Update()
{
	// 점수 디스플레이는 특별한 업데이트가 필요 없음
	// 필요시 애니메이션 등을 추가할 수 있음
}
