#include "헤더.h"
#include "WindowToNDC.h"
#include "shader_func.h"

void InitBuffer();

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
//GLvoid SpecialKeyDown(int key, int x, int y);
//GLvoid SpecialKeyUp(int key, int x, int y);

// 전역 변수 정의 (CommonHeaders.h에서 extern으로 선언된 것들)
// shaderProgramID는 shader_func.h에서 이미 정의됨
glm::mat4 gProjection(1.0f);
glm::mat4 gView(1.0f);
glm::mat4 gModel(1.0f);
GLint uMVP_loc = -1;


//--- 메인 함수
void main(int argc, char** argv)
//--- 윈도우출력하고콜백함수설정
{
	GLint width, height;

	width = 1600;
	height = 900;
	//--- 윈도우생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);		//GLUT_DEPTH 깊이에 따른 은면제거
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example17");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	//--- 세이더읽어와서세이더프로그램만들기: 사용자정의함수호출
	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();
	AfterMakeShaders();	//셰이더에서 uniform 변수 위치 얻기

	glutReshapeFunc(Reshape);
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyboard);
	//glutSpecialFunc(SpecialKeyDown);    // 화살표 등 특수키 눌림 처리
	//glutSpecialUpFunc(SpecialKeyUp);
	glutTimerFunc(16, Timer, 1); // 약 60FPS로 타이머 시작

	InitBuffer();
	glutMainLoop();
}

void InitBuffer()
{
	glEnable(GL_DEPTH_TEST); // 깊이버퍼 활성화
	glEnable(GL_CULL_FACE);

	//RectInit();<< 따로 초기화 함수 만들어서 호출만 하기 
}

//--- 출력 콜백함수
GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//GL_DEPTH_BUFFER_BIT 깊이에 따른 은면제거


	//--- 렌더링 파이프라인에 세이더 불러우기
	glUseProgram(shaderProgramID);

	// --- View: 카메라를 뒤쪽 위쪽에서 원점을 바라보도록 설정
	gView = glm::mat4(1.0f);
	gView = glm::lookAt(		//카메라 외부파라미터
		glm::vec3(-1.0f, 2.0f, 3.0f),  // 카메라 위치 (x, y, z축이 모두 보이는 위치)	EYE
		glm::vec3(0.0f, 0.0f, 0.0f),  // 바라보는 지점 (원점) 							AT
		glm::vec3(0.0f, 1.0f, 0.0f)   // 위쪽 방향 벡터 					 			UP
	);

	//RectDraw();<< 따로 렌더 함수 만들어서 호출만 하기 

	glutSwapBuffers();
}

//--- 다시그리기콜백함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	glUseProgram(shaderProgramID);

	float aspect = (h == 0) ? 1 : (float)w / (float)h;

	//원근 투영 사용 (3D 효과를 더 잘 보여줌)
	gProjection = glm::perspective(
		glm::radians(45.0f),  // 시야각 45도	fovy
		aspect,               // 종횡비			aspect
		0.1f,                 // 근평면			-n
		100.0f                // 원평면			-f
	);

	glEnable(GL_DEPTH_TEST); // 깊이버퍼 활성화
	glUseProgram(0);
}

GLvoid Timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1); // 약 60FPS로 타이머 시작
}


GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {

	case 'q':
	case 'Q':
		exit(0);
		break;
	}

	glutPostRedisplay();
}