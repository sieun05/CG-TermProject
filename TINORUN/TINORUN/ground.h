#pragma once
#include "Header.h"
#include "game_object.h"

class Ground : public GameObject {
public:
	Ground() = default;
	Ground(int round, RGBA color) : round(round), color(color) {}
	Ground(int round, RGBA color, const std::string& texturePath);
	~Ground() = default;
	Ground(const Ground& other) = default;
	Ground& operator=(const Ground& other) = default;

	int round;
	RGBA color;

	bool LoadTexture(const std::string& texturePath);

	// GameObject�� ���� �Լ����� �������̵�
	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;

private:
	GLuint textureID = 0;
	bool useTexture = false;
};

extern GLuint VAO_ground;
extern GLuint VBO_ground[2];
extern GLuint EBO_ground;
extern GLuint EBO_ground_lines; // �𼭸��� EBO �߰�

void GroundInit();
void ChangeGroundColor(RGBA newColor);