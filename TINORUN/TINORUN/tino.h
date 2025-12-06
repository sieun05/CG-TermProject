#pragma once
#include "Header.h"
#include "game_object.h"
#include "common.h"
#include <string>

enum State {
	RUNNING,
	JUMPING,
	SLIDING
};

class Tino : public GameObject {
public:
    Tino() = default;
    Tino(const std::string& objPath, const std::string& jumpPath,
        const std::string& downPath, const std::string& texturePath);
    ~Tino();
    Tino(const Tino& other) = delete;
    Tino& operator=(const Tino& other) = delete;

    // 렌더링
    void Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc) override;
    void Update() override;
    
    void StateChange(State newState);

    // 로드
    bool LoadOBJ(const std::string& objPath, State targetState);
    bool LoadTexture(const std::string& texturePath);

private:
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        bool isLoaded;

        MeshData() : VAO(0), VBO(0), EBO(0), isLoaded(false) {}
    };

    MeshData meshes[3];

    GLuint textureID;
    BITMAPINFO* bmp;

    State state;
    float stateTimer;

	static constexpr float JUMP_DURATION = 0.5f; // 점프 상태 지속 시간 (초)
	static constexpr float SLIDE_DURATION = 0.5f; // 슬라이드 상태 지속 시간 (초)

    void SetupMesh(State targetState);
};

// ��ƿ��Ƽ �Լ���
// GLuint LoadTextureFromFile(const std::string& path);

