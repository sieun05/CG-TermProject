#pragma once
#include "game_object.h"

class Button : public GameObject {
public:
	Button() = default;
	Button(float x, float y, float w, float h, const std::string& texturePath);
	~Button() = default;

	// 버퍼 초기화 및 텍스처 로드
	void InitBuffer();
	bool LoadTexture(const std::string& texturePath);

	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;
private:
	float x, y, w, h;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint textureID;
	BITMAPINFO* bmp;
};

