#include "ground.h"
#include "game_state.h"
#include "LoadBitmap.h"

extern "C" void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

GLuint VAO_ground{};
GLuint VBO_ground[2]{};
GLuint EBO_ground{};
GLuint EBO_ground_lines{}; // �𼭸��� EBO �߰�

Ground::Ground(int round, RGBA color, const std::string& texturePath)
    : round(round), color(color)
{
    useTexture = LoadTexture(texturePath);
}

bool Ground::LoadTexture(const std::string& texturePath)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    if (data == NULL) {
        std::cerr << "Failed to load ground texture: " << texturePath << std::endl;
        return false;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    return true;
}

void GroundInit()
{
    const float size = 2.0f; // ������ü �� ���� ���� ���� (�߽��� ����)

    const float ground_vertices[] = {
        // �ո� (z = +size) - ��ġ + �ؽ�ó ��ǥ
        -size, -size,  size,  0.0f, 0.0f,  // 0
         size, -size,  size,  1.0f, 0.0f,  // 1
         size,  size,  size,  1.0f, 1.0f,  // 2
        -size,  size,  size,  0.0f, 1.0f,  // 3

        // �޸� (z = -size)
         size, -size, -size,  0.0f, 0.0f,  // 4
        -size, -size, -size,  1.0f, 0.0f,  // 5
        -size,  size, -size,  1.0f, 1.0f,  // 6
         size,  size, -size,  0.0f, 1.0f,  // 7

        // ���ʸ� (x = -size)
        -size, -size, -size,  0.0f, 0.0f,  // 8
        -size, -size,  size,  1.0f, 0.0f,  // 9
        -size,  size,  size,  1.0f, 1.0f,  // 10
        -size,  size, -size,  0.0f, 1.0f,  // 11

        // �����ʸ� (x = +size)
         size, -size,  size,  0.0f, 0.0f,  // 12
         size, -size, -size,  1.0f, 0.0f,  // 13
         size,  size, -size,  1.0f, 1.0f,  // 14
         size,  size,  size,  0.0f, 1.0f,  // 15

        // �Ʒ��� (y = -size)
        -size, -size, -size,  0.0f, 0.0f,  // 16
         size, -size, -size,  1.0f, 0.0f,  // 17
         size, -size,  size,  1.0f, 1.0f,  // 18
        -size, -size,  size,  0.0f, 1.0f,  // 19

        // ���� (y = +size)
        -size,  size,  size,  0.0f, 0.0f,  // 20
         size,  size,  size,  1.0f, 0.0f,  // 21
         size,  size, -size,  1.0f, 1.0f,  // 22
        -size,  size, -size,  0.0f, 1.0f   // 23

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
        // �ո� (Red) - �ݽð����
        0, 1, 2,   2, 3, 0,

        // �޸� (Green) - �ݽð����
        4, 5, 6,   6, 7, 4,

        // ���ʸ� (Blue) - �ݽð����
        8, 9, 10,   10, 11, 8,

        // �����ʸ� (Yellow) - �ݽð����
        12, 13, 14,   14, 15, 12,

        // �Ʒ��� (Magenta) - �ݽð����
        16, 17, 18,   18, 19, 16,

        // ���� (Cyan) - �ݽð����
        20, 21, 22,   22, 23, 20
    };

    // ������ü�� 12�� �𼭸� �ε��� (8�� ���� ����)
    const unsigned int ground_line_indices[] = {
        // �ո��� 4�� �𼭸�
        0, 1,   1, 2,   2, 3,   3, 0,
        // �޸��� 4�� �𼭸�  
        4, 5,   5, 6,   6, 7,   7, 4,
        // �յڸ� �����ϴ� 4�� �𼭸�
        0, 5,   1, 4,   2, 7,   3, 6
    };

    // VAO ���� �� ���ε�
    glGenVertexArrays(1, &VAO_ground);
    glBindVertexArray(VAO_ground);

    // VBO ����
    glGenBuffers(2, VBO_ground);

    // ���� ��ġ + �ؽ�ó ��ǥ ������
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ground[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);

    // ��ġ �Ӽ� (location 0) - stride�� 5*sizeof(float)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // �ؽ�ó ��ǥ �Ӽ� (location 2) - offset�� 3*sizeof(float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // ���� ���� ������
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ground[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground_colors), ground_colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // EBO (�ε��� ����) ���� - �ﰢ����
    glGenBuffers(1, &EBO_ground);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_indices), ground_indices, GL_STATIC_DRAW);

    // EBO (�ε��� ����) ���� - �𼭸���
    glGenBuffers(1, &EBO_ground_lines);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground_lines);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_line_indices), ground_line_indices, GL_STATIC_DRAW);

    // ���ε� ����
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
	if (!useTexture) {
		ChangeGroundColor(this->color);
	}

    glBindVertexArray(VAO_ground);

    // �θ� Ŭ������ GetModelMatrix() ���
    glm::mat4 model = GetModelMatrix();

    glm::mat4 mvp = gProjection * gView * model;
    glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

    // Update model matrix for lighting
    if (uModel_loc >= 0) {
        glUniformMatrix4fv(uModel_loc, 1, GL_FALSE, &model[0][0]);
    }

	// �ؽ�ó ��� ����
	if (useTexture) {
		glUniform1i(uUseTexture_loc, 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(uTextureSampler_loc, 0);
	} else {
		glUniform1i(uUseTexture_loc, 0);
	}

    // ���� ���� �׸� (ä���� �ﰢ��)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // ������ �𼭸��� �׸� (�����θ�)
    RGBA blackColor = {0.0f, 0.0f, 0.0f, 1.0f};
    ChangeGroundColor(blackColor);

    glLineWidth(2.0f); // �� �β� ����

    // �𼭸��� EBO�� �����ϰ� ������ �׸���
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_ground_lines);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0); // 12�� �𼭸� * 2�� �� = 24�� �ε���

    glLineWidth(1.0f); // �� �β� ����

	if (useTexture) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(uUseTexture_loc, 0);
	}

    glBindVertexArray(0);
}

void Ground::Update()
{
    return;
}