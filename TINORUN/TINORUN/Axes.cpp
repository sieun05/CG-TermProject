#include "Axes.h"

// 좌표축 관련 전역 변수 정의
GLuint VAO_axes = 0;
GLuint VBO_axes[2] = { 0, };

void InitAxesBuffer() {
	const float L = 200.0f;

	// [x,y,z] * 6개 정점 (GL_LINES이므로 2개 = 1개 선분)
	const float axes_vertices[] = {
		// X axis (red)
		-L, 0.0f, 0.0f,   
		 L, 0.0f, 0.0f,   

		 // Y axis (green)
		  0.0f, -L, 0.0f, 
		  0.0f,  L, 0.0f, 

		  // Z axis (blue)
		   0.0f, 0.0f, -L,
		   0.0f, 0.0f,  L,
	};

	const float axes_colors[] = {
		// X axis (red)
		1.0f, 0.0f, 0.0f, // start point
		1.0f, 0.0f, 0.0f, // end point

		// Y axis (green)
		0.0f, 1.0f, 0.0f, // start point
		0.0f, 1.0f, 0.0f, // end point

		// Z axis (blue)
		0.0f, 0.0f, 1.0f, // start point
		0.0f, 0.0f, 1.0f, // end point
	};

	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glGenBuffers(2, VBO_axes);

	// positions -> location=0
	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), axes_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// colors -> location=1
	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_colors), axes_colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void DrawAxes(glm::mat4 gProjection, glm::mat4 gView, GLuint uMVP_loc)
{

	glBindVertexArray(VAO_axes);

	// 모델 행렬 (좌표축은 원점에 고정)
	glm::mat4 model = glm::mat4(1.0f);
	
	// MVP 행렬 계산
	glm::mat4 mvp = gProjection * gView * model;

	// 유니폼 설정
	glUniformMatrix4fv(uMVP_loc, 1, GL_FALSE, &mvp[0][0]);

	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, 6); // 6개의 정점 = 3개의 선분
	glLineWidth(1.0f); // 선 두께 복원
	
	glBindVertexArray(0);
}