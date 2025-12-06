#pragma once
#include "game_object.h"
#include <string>

class ScoreDisplay : public GameObject {
public:
	ScoreDisplay();
	ScoreDisplay(float x, float y, float digitWidth, float digitHeight, const std::string& texturePath);
	~ScoreDisplay();

	// 버퍼 초기화 및 텍스처 로드
	void InitBuffer();
	bool LoadTexture(const std::string& texturePath);

	// 점수 설정
	void SetScore(int score);

	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;

private:
	// 한 자리 숫자를 그리는 함수 (실제 사용된 너비를 반환)
	float DrawDigit(int digit, float x, float y, glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc);

	int currentScore;      // 현재 점수
	float digitWidth;      // 각 숫자의 너비
	float digitHeight;     // 각 숫자의 높이
	float spacing;         // 숫자 간격

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint textureID;
};
