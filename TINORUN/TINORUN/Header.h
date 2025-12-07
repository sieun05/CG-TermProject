#pragma once
#define _CRT_SECURE_NO_WARNINGS 
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

// 함수 선언만 (구현은 별도 파일에서)
void PostScoreToServer(const char* id, int score);

using namespace std;

// 색상 구조체 및 타입 정의
struct RGBA { float r, g, b, a; };

struct Boundary {
	glm::vec3 r1;
	glm::vec3 r2;
	glm::vec3 r3;
	glm::vec3 r4;
	glm::vec3 r5;
	glm::vec3 r6;
};

// Global matrices (shaderProgramID is defined in shader_func.h)
extern glm::mat4 gProjection;
extern glm::mat4 gView;
extern glm::mat4 gModel;
extern GLint uMVP_loc;

// 변환 행렬 관련 uniform 변수들
extern GLint uModel_loc;
extern GLint uView_loc;
extern GLint uProjection_loc;

// 텍스처 관련 uniform 변수들
extern GLint uUseTexture_loc;
extern GLint uTextureSampler_loc;

// 조명 관련 uniform 변수들
extern GLint uUseLighting_loc;

// 함수 선언
void InitGameObjects();