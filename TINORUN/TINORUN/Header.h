#pragma once
#define _CRT_SECURE_NO_WARNINGS 
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// ���� ����ü �� Ÿ�� ����
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

// Texture uniform locations
extern GLint uUseTexture_loc;
extern GLint uTextureSampler_loc;

// Lighting and shadow uniform locations
extern GLint uModel_loc;
extern GLint uView_loc;
extern GLint uProjection_loc;
extern GLint uLightSpaceMatrix_loc;
extern GLint uUseLighting_loc;
extern GLint uUseShadows_loc;
extern GLint uLightDir_loc;
extern GLint uLightColor_loc;
extern GLint uViewPos_loc;
extern GLint uAmbientStrength_loc;
extern GLint uSpecularStrength_loc;
extern GLint uShininess_loc;
extern GLint uShadowMap_loc;

// Shadow system
extern GLuint shadowMapFBO;
extern GLuint shadowMapTexture;
extern const unsigned int SHADOW_WIDTH;
extern const unsigned int SHADOW_HEIGHT;
extern GLuint shadowShaderProgram;
extern glm::mat4 lightSpaceMatrix;