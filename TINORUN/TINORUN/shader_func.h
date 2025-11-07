#pragma once
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h> 

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
void AfterMakeShaders();

//--- 필요한변수선언
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader;	//--- 버텍스세이더객체
GLuint fragmentShader;	//--- 프래그먼트 세이더객체

void AfterMakeShaders()
{
	glUseProgram(shaderProgramID);
	uMVP_loc = glGetUniformLocation(shaderProgramID, "uMVP");
	if (uMVP_loc < 0) { printf("uMVP get error\n"); exit(1); }
	glUseProgram(0);
}

char* filetobuf(const char* file)	//쉐이더 파일을 읽어 문자열로 변환하는 유틸리티 함수
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


//--- 버텍스세이더객체만들기
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스세이더읽어저장하고컴파일하기
	//--- filetobuf: 사용자정의 함수로 텍스트를읽어서문자열에저장하는함수

	vertexSource = filetobuf("vertex.glsl");

	//버텍스 쉐이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//쉐이더 코드를 쉐이더 객체에 넣기
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	//버텍스 쉐이더 컴파일하기
	glCompileShader(vertexShader);

	//컴파일이 제대로 되었는지 체크하기
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
//--- 프래그먼트세이더객체만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트세이더읽어저장하고컴파일하기
	fragmentSource = filetobuf("fragment.glsl");    // 프래그세이더 읽어오기

	//--- 프래그먼트세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 쉐이더코드를쉐이더객체에넣기
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	//--- 프래그먼트세이더컴파일하기
	glCompileShader(fragmentShader);

	//--- 컴파일이제대로되었는지체크하기
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
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
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return -1;
	}

	glUseProgram(shaderID);
	return shaderID;
}
