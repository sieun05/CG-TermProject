#include "ground.h"
#include "game_state.h"

GLuint VAO_ground{};
GLuint VBO_ground[2]{};
GLuint EBO_ground{};
GLuint EBO_ground_lines{}; // 모서리용 EBO 추가

void GroundInit()
{
    const float size = 2.0f; // 정육면체 한 변의 절반 길이 (중심이 원점)

    const float ground_vertices[] = {
        // 앞면 (z = +size) - 0~3 (바깥쪽에서 봤을 때 반시계방향)
        -size, -size,  size,  // 0: 왼쪽 아래
         size, -size,  size,  // 1: 오른쪽 아래
         size,  size,  size,  // 2: 오른쪽 위
        -size,  size,  size,  // 3: 왼쪽 위

        // 뒷면 (z = -size) - 4~7 (바깥쪽에서 봤을 때 반시계방향)
         size, -size, -size,  // 4: 오른쪽 아래 (뒤에서 보면 왼쪽)
        -size, -size, -size,  // 5: 왼쪽 아래 (뒤에서 보면 오른쪽)
        -size,  size, -size,  // 6: 왼쪽 위 (뒤에서 보면 오른쪽)
         size,  size, -size,  // 7: 오른쪽 위 (뒤에서 보면 왼쪽)

         // 왼쪽면 (x = -size) - 8~11 (바깥쪽에서 봤을 때 반시계방향)
         -size, -size, -size,  // 8: 뒤 아래
         -size, -size,  size,  // 9: 앞 아래
         -size,  size,  size,  // 10: 앞 위
         -size,  size, -size,  // 11: 뒤 위

         // 오른쪽면 (x = +size) - 12~15 (바깥쪽에서 봤을 때 반시계방향)
          size, -size,  size,  // 12: 앞 아래
          size, -size, -size,  // 13: 뒤 아래
          size,  size, -size,  // 14: 뒤 위
          size,  size,  size,  // 15: 앞 위

          // 아래면 (y = -size) - 16~19 (바깥쪽에서 봤을 때 반시계방향)
          -size, -size, -size,  // 16: 왼쪽 뒤
           size, -size, -size,  // 17: 오른쪽 뒤
           size, -size,  size,  // 18: 오른쪽 앞
          -size, -size,  size,  // 19: 왼쪽 앞

          // 위면 (y = +size) - 20~23 (바깥쪽에서 봤을 때 반시계방향)
          -size,  size,  size,  // 20: 왼쪽 앞
           size,  size,  size,  // 21: 오른쪽 앞
           size,  size, -size,  // 22: 오른쪽 뒤
          -size,  size, -size   // 23: 왼쪽 뒤

    };

    const float ground_colors[] = {
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    const unsigned int ground_indices[] = {
        // 앞면 (Red) - 반시계방향
        0, 1, 2,   2, 3, 0,

        // 뒷면 (Green) - 반시계방향
        4, 5, 6,   6, 7, 4,

        // 왼쪽면 (Blue) - 반시계방향
        8, 9, 10,   10, 11, 8,

        // 오른쪽면 (Yellow) - 반시계방향
        12, 13, 14,   14, 15, 12,

        // 아래면 (Magenta) - 반시계방향
        16, 17, 18,   18, 19, 16,

        // 위면 (Cyan) - 반시계방향
        20, 21, 22,   22, 23, 20
    };

    // 정육면체의 12개 모서리 인덱스 (8개 정점 기준)
    const unsigned int ground_line_indices[] = {
        // 앞면의 4개 모서리
        0, 1,   1, 2,   2, 3,   3, 0,
        // 뒷면의 4개 모서리  
        4, 5,   5, 6,   6, 7,   7, 4,
        // 앞뒤를 연결하는 4개 모서리
        0, 5,   1, 4,   2, 7,   3, 6
    };

    // VAO 생성 및 바인딩
    glGenVertexArrays(1, &VAO_ground);
    glBindVertexArray(VAO_ground);

    // VBO 생성
    glGenBuffers(2, VBO_ground);

    // 정점 위치 데이터
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ground[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 정점 색상 데이터
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ground[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_colors), ground_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // EBO (인덱스 버퍼) 설정 - 삼각형용
    glGenBuffers(1, &EBO_ground);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_indices), ground_indices, GL_STATIC_DRAW);

    // EBO (인덱스 버퍼) 설정 - 모서리용
    glGenBuffers(1, &EBO_ground_lines);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground_lines);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_line_indices), ground_line_indices, GL_STATIC_DRAW);

    // 바인딩 해제
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void ChangeGroundColor(RGBA newColor)
{
    float ground_colors[] = {
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,

        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,

        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,

        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,

        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,

        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
        newColor.r, newColor.g, newColor.b,
	};
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ground[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_colors), ground_colors, GL_STATIC_DRAW);
}

void Ground::Draw(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{
    if (scene == GameState::TITLE) return;

	ChangeGroundColor(this->color);

    glBindVertexArray(VAO_ground);
    
    // 부모 클래스의 GetModelMatrix() 사용
    glm::mat4 model = GetModelMatrix();
    
    // x방향으로 100배, y방향으로 0.5배, z방향으로 1.5배 스케일링 적용
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 0.3f, 1.3f));
    model = model * scaleMatrix;

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // 먼저 면을 그림 (채워진 삼각형)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    // 검정색 모서리를 그림 (선으로만)
    RGBA blackColor = {0.0f, 0.0f, 0.0f, 1.0f};
    ChangeGroundColor(blackColor);
    
    glLineWidth(2.0f); // 선 두께 설정
    
    // 모서리용 EBO로 변경하고 선으로 그리기
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground_lines);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0); // 12개 모서리 * 2개 점 = 24개 인덱스
    
    glLineWidth(1.0f); // 선 두께 복원
    
    glBindVertexArray(0);
}

void Ground::Update()
{
    return;
}