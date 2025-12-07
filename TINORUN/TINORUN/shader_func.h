#pragma once
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void AfterMakeShaders();

// Shadow shader 함수들
void make_shadowVertexShaders();
void make_shadowFragmentShaders();
GLuint make_shadowShaderProgram();
void InitShadowMap();

//--- 필요한변수들
extern GLuint shaderProgramID; //--- 셰이더 프로그램 이름
extern GLuint vertexShader;	//--- 버텍스셰이더객체
extern GLuint fragmentShader;	//--- 프래그먼트 셰이더객체

// Shadow shader 변수들
extern GLuint shadowVertexShader;
extern GLuint shadowFragmentShader;

// 기본 uniform 변수 위치들
extern GLint uMVP_loc;
extern GLint uModel_loc;
extern GLint uView_loc;
extern GLint uProjection_loc;
extern GLint uUseTexture_loc;
extern GLint uTextureSampler_loc;
extern GLint uUseLighting_loc;
extern GLint uShadowMap_loc;

// Shadow system extern declarations
extern GLuint shadowMapFBO;
extern GLuint shadowMapTexture;
extern const unsigned int SHADOW_WIDTH;
extern const unsigned int SHADOW_HEIGHT;
extern GLuint shadowShaderProgram;

inline void AfterMakeShaders()
{
	glUseProgram(shaderProgramID);

	// 기본 변환 행렬들
	uMVP_loc = glGetUniformLocation(shaderProgramID, "uMVP");
	if (uMVP_loc < 0) { printf("uMVP get error\n"); }

	uModel_loc = glGetUniformLocation(shaderProgramID, "uModel");
	if (uModel_loc < 0) { printf("uModel get error\n"); }

	// uView와 uProjection은 선택적 (사용되지 않으면 -1 반환)
	uView_loc = glGetUniformLocation(shaderProgramID, "uView");
	uProjection_loc = glGetUniformLocation(shaderProgramID, "uProjection");

	// 텍스처 관련 uniform 변수 위치 얻기
	uUseTexture_loc = glGetUniformLocation(shaderProgramID, "useTexture");
	uTextureSampler_loc = glGetUniformLocation(shaderProgramID, "textureSampler");

	// 조명 관련 uniform
	uUseLighting_loc = glGetUniformLocation(shaderProgramID, "useLighting");

	// 텍스처 샘플러를 텍스처 유닛 0에 바인딩
	if (uTextureSampler_loc >= 0) {
		glUniform1i(uTextureSampler_loc, 0);  // Texture unit 0
	}
	if (uShadowMap_loc >= 0) {
		glUniform1i(uShadowMap_loc, 1);  // Texture unit 1
	}

	// 기본적으로 조명 활성화
	if (uUseLighting_loc >= 0) {
		glUniform1i(uUseLighting_loc, 1);
	}

	glUseProgram(0);
}

inline char* filetobuf(const char* file)	//���̴� ������ �о� ���ڿ��� ��ȯ�ϴ� ��ƿ��Ƽ �Լ�
{
	FILE* fptr;
	long length;
	char* buf;

	fptr = fopen(file, "rb");

	if (!fptr)
		return NULL;

	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;

	return buf;
}

//--- ���ؽ����̴���ü�����
inline void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스세이더읽어저장하고컴파일하기
	//--- filetobuf: 파일내용을 읽는함수는 텍스트를읽어서문자열에저장하는함수

	vertexSource = filetobuf("vertex.glsl");

	//���ؽ� ���̴� ��ü �����
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	//���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexShader);

	//�������� ����� �Ǿ����� üũ�ϱ�
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}
//--- �����׸�Ʈ���̴���ü�����
inline void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ���̴��о������ϰ��������ϱ�
	fragmentSource = filetobuf("fragment.glsl");    // �����׼��̴� �о����

	//--- 프래그먼트세이더객체만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- ���̴��ڵ带���̴���ü���ֱ�
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	//--- �����׸�Ʈ���̴��������ϱ�
	glCompileShader(fragmentShader);

	//--- ������������εǾ�����üũ�ϱ�
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

inline GLuint make_shaderProgram()
{
	GLint result;
	GLchar errorLog[512];
	GLuint shaderID;
	shaderID = glCreateProgram();

	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	glLinkProgram(shaderID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);

	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return -1;
	}

	glUseProgram(shaderID);
	return shaderID;
}

// Shadow shader functions
inline void make_shadowVertexShaders()
{
	GLchar* vertexSource;
	vertexSource = filetobuf("shadow_vertex.glsl");

	shadowVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shadowVertexShader, 1, &vertexSource, NULL);
	glCompileShader(shadowVertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(shadowVertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(shadowVertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: shadow vertex shader compile error\n" << errorLog << std::endl;
		return;
	}
}

inline void make_shadowFragmentShaders()
{
	GLchar* fragmentSource;
	fragmentSource = filetobuf("shadow_fragment.glsl");

	shadowFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shadowFragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(shadowFragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(shadowFragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(shadowFragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: shadow fragment shader compile error\n" << errorLog << std::endl;
		return;
	}
}

inline GLuint make_shadowShaderProgram()
{
	GLint result;
	GLchar errorLog[512];
	GLuint shaderID;
	shaderID = glCreateProgram();

	glAttachShader(shaderID, shadowVertexShader);
	glAttachShader(shaderID, shadowFragmentShader);
	glLinkProgram(shaderID);

	glDeleteShader(shadowVertexShader);
	glDeleteShader(shadowFragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);

	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shadow shader program link error\n" << errorLog << std::endl;
		return -1;
	}

	return shaderID;
}

inline void InitShadowMap()
{
	// Generate framebuffer
	glGenFramebuffers(1, &shadowMapFBO);

	// Generate depth texture
	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// Check framebuffer status
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR: Shadow framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}