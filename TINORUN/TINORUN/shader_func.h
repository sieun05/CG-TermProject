#pragma once
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void AfterMakeShaders();

// Shadow shader functions
void make_shadowVertexShaders();
void make_shadowFragmentShaders();
GLuint make_shadowShaderProgram();
void InitShadowMap();

// Required variables
GLuint shaderProgramID; // Shader program name
GLuint vertexShader;	// Vertex shader object
GLuint fragmentShader;	// Fragment shader object

// Shadow shader objects
GLuint shadowVertexShader;
GLuint shadowFragmentShader;

// Uniform location variables
extern GLint uUseTexture_loc;
extern GLint uTextureSampler_loc;

void AfterMakeShaders()
{
	glUseProgram(shaderProgramID);

	// Basic uniform locations
	uMVP_loc = glGetUniformLocation(shaderProgramID, "uMVP");
	if (uMVP_loc < 0) { printf("uMVP get error\n"); exit(1); }

	// Texture uniform locations
	uUseTexture_loc = glGetUniformLocation(shaderProgramID, "useTexture");
	uTextureSampler_loc = glGetUniformLocation(shaderProgramID, "textureSampler");

	// Lighting and shadow uniform locations
	uModel_loc = glGetUniformLocation(shaderProgramID, "uModel");
	uView_loc = glGetUniformLocation(shaderProgramID, "uView");
	uProjection_loc = glGetUniformLocation(shaderProgramID, "uProjection");
	uLightSpaceMatrix_loc = glGetUniformLocation(shaderProgramID, "uLightSpaceMatrix");
	uUseLighting_loc = glGetUniformLocation(shaderProgramID, "useLighting");
	uUseShadows_loc = glGetUniformLocation(shaderProgramID, "useShadows");
	uLightDir_loc = glGetUniformLocation(shaderProgramID, "lightDir");
	uLightColor_loc = glGetUniformLocation(shaderProgramID, "lightColor");
	uViewPos_loc = glGetUniformLocation(shaderProgramID, "viewPos");
	uAmbientStrength_loc = glGetUniformLocation(shaderProgramID, "ambientStrength");
	uSpecularStrength_loc = glGetUniformLocation(shaderProgramID, "specularStrength");
	uShininess_loc = glGetUniformLocation(shaderProgramID, "shininess");
	uShadowMap_loc = glGetUniformLocation(shaderProgramID, "shadowMap");

	// Bind texture samplers to texture units
	if (uTextureSampler_loc >= 0) {
		glUniform1i(uTextureSampler_loc, 0);  // Texture unit 0
	}
	if (uShadowMap_loc >= 0) {
		glUniform1i(uShadowMap_loc, 1);  // Texture unit 1
	}

	glUseProgram(0);
}

char* filetobuf(const char* file)	//���̴� ������ �о� ���ڿ��� ��ȯ�ϴ� ��ƿ��Ƽ �Լ�
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
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- ���ؽ����̴��о������ϰ��������ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ���о���ڿ��������ϴ��Լ�

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
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ���̴��о������ϰ��������ϱ�
	fragmentSource = filetobuf("fragment.glsl");    // �����׼��̴� �о����

	//--- �����׸�Ʈ���̴� ��ü �����
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

GLuint make_shaderProgram()
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
void make_shadowVertexShaders()
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

void make_shadowFragmentShaders()
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

GLuint make_shadowShaderProgram()
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

void InitShadowMap()
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
