#pragma once
#include "Header.h"
#include "game_object.h"

class Ground : public GameObject {
public:
	Ground() = default;
	Ground(int round, RGBA color) : round(round), color(color) {}
	~Ground() = default;
	Ground(const Ground& other) = default;
	Ground& operator=(const Ground& other) = default;

	int round;
	RGBA color;

	// GameObject의 가상 함수들을 오버라이드
	void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
	void Update() override;
};

extern GLuint VAO_ground;
extern GLuint VBO_ground[2];
extern GLuint EBO_ground;
extern GLuint EBO_ground_lines; // 모서리용 EBO 추가

void GroundInit();
void ChangeGroundColor(RGBA newColor);