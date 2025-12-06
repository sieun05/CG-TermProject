#pragma once
#include "Header.h"

class GameObject {
public:
	GameObject() = default;
	virtual ~GameObject() = default;  // 가상 소멸자로 변경
	GameObject(const GameObject& other) = default;
	GameObject& operator=(const GameObject& other) = default;

	// 순수 가상 함수로 만들어 자식 클래스에서 반드시 구현하도록 함
	virtual void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) = 0;
	virtual void Update() = 0;
	virtual void OnCollision(GameObject* other) {}

	// 위치와 회전 등 공통 속성들
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	Boundary boundary{}; // 경계 박스

	// 활성화 상태
	bool isActive = true;

protected:
	// 공통으로 사용할 모델 변환 매트릭스 계산 함수
	glm::mat4 GetModelMatrix() const {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scale);
		return model;
	}

private:

};