#pragma once
#include "game_object.h"

class Images : public GameObject {
public:
	Images() = default;
	Images(float x, float y, float z, float w, float h, const std::string& texturePath);
	~Images() = default;

	// 버퍼 초기화 및 텍스처 로드
	void InitBuffer();
	bool LoadTexture(const std::string& texturePath);

	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;
private:
	float x, y, z, w, h;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint textureID;
	BITMAPINFO* bmp;

	float OffsetX = 0.0f;
	float moveSpeed = 0.0005f;
};

