#pragma once
#include "헤더.h"

// 좌표축 관련 전역 변수
extern GLuint VAO_axes;
extern GLuint VBO_axes[2];

// 좌표축 관련 함수 선언
void InitAxesBuffer();
void DrawAxes();