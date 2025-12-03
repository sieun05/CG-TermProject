#include "헤더.h"
#include "WindowToNDC.h"
#include "shader_func.h"
#include "ground.h"
#include "game_world.h"
#include "tino.h"  // Tino 헤더 추가

#include "Axes.h"

void InitBuffer();
void InitGameObjects();

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

// 텍스처 관련 uniform 변수 정의
GLint uUseTexture_loc = -1;
GLint uTextureSampler_loc = -1;

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
	InitGameObjects();		// 게임 객체 초기화
	glutMainLoop();
}

void InitBuffer()
{
	glEnable(GL_DEPTH_TEST); // 깊이버퍼 활성화
	glEnable(GL_CULL_FACE);

	//RectInit();<< 따로 초기화 함수 만들어서 호출만 하기 
	GroundInit();
	InitAxesBuffer();
}

void InitGameObjects()
{
	// Ground 객체 생성 및 GameWorld에 추가
	auto ground = std::make_unique<Ground>(1, RGBA{ 231/255., 217/255., 176/255., 1.0f });
	g_gameWorld.AddObject(std::move(ground));

	// Tino 객체 생성 및 GameWorld에 추가
	// 경로 수정: assets 폴더로 직접 접근
	auto tino = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_base.png");
	tino->position = glm::vec3(0.0f, 1.0f, 0.0f);  // Ground 위에 배치
	tino->scale = glm::vec3(1.0f, 1.0f, 1.0f);     // 크기 조정 (우선 기본 크기로)
	g_gameWorld.AddObject(std::move(tino));
}

//--- 출력 콜백함수
GLvoid drawScene()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//GL_DEPTH_BUFFER_BIT 깊이에 따른 은면제거


	//--- 렌더링 파이프라인에 세이더 불러우기
	glUseProgram(shaderProgramID);

	DrawAxes(); // 좌표축 그리기

	// --- View: 카메라를 뒤쪽 위쪽에서 원점을 바라보도록 설정
	gView = glm::mat4(1.0f);
	gView = glm::lookAt(		//카메라 외부파라미터
		glm::vec3(-10.0f, 6.0f, 7.0f),  // 카메라 위치 (x, y, z축이 모두 보이는 위치)	EYE
		glm::vec3(0.0f, 0.0f, -3.0f),  // 바라보는 지점 (원점) 							AT
		glm::vec3(0.0f, 1.0f, 0.0f)   // 위쪽 방향 벡터 					 			UP
	);

	// GameWorld를 통해 모든 객체 렌더링
	g_gameWorld.DrawAll(gProjection, gView, uMVP_loc);

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
	// GameWorld를 통해 모든 객체 업데이트
	g_gameWorld.UpdateAll();
	
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1); // 약 60FPS로 타이머 시작
}


GLvoid Keyboard(unsigned char key, int x, int y)
{
	const float deltaTime = 0.016f; // 약 60FPS 기준

	switch (key) {

	case 'q':
	case 'Q':
		exit(0);
		break;
	}

	glutPostRedisplay();
}