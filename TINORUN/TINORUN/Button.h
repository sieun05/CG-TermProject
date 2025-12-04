#pragma once
#include "game_object.h"

class Button : public GameObject {
public:
	Button() = default;
	Button(float x, float y, float w, float h);
	~Button() = default;
	void InitBuffer();
	void InitTexture();
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

