#include "Header.h"
#include "WindowToNDC.h"
#include "shader_func.h"
#include "game_state.h"
#include "game_world.h"
#include "ground.h"
#include "tino.h"  // Tino 헤더 추가
#include "obstacle.h" // 장애물 헤더 추가 
#include "Images.h"	// 버튼 헤더 추가
#include "ScoreDisplay.h"	// 점수 헤더 추가

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#include "Axes.h"

void InitBuffer();
void InitGameObjects();

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
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

Tino* tino = nullptr;
ScoreDisplay* scoreDisplay = nullptr;
int gameScore = 0;

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
	glutInitWindowPosition(100, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("TINO RUN");

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
	glutMouseFunc(Mouse);
	//glutSpecialFunc(SpecialKeyDown);    // 화살표 등 특수키 눌림 처리
	//glutSpecialUpFunc(SpecialKeyUp);
	glutTimerFunc(16, Timer, 1); // 약 60FPS로 타이머 시작

	InitBuffer();
	InitGameObjects();		// 게임 객체 초기화
	
	// 게임 상태를 PLAYING으로 설정 (테스트용)
	scene = GameState::TITLE;
	
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
	if (scene == GameState::TITLE) {
		g_gameWorld.Clear(); // 이전 게임 객체들 제거

		auto press_enter = std::move(std::make_unique<Images>(0.5f, -1.0f, 0.8f, 0.3f, "assets/Press_Enter.png"));
		g_gameWorld.AddObject(std::move(press_enter));

		// Ground 객체 생성 및 GameWorld에 추가
		auto ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f });
		ground->position.y = -4.0f; // 땅을 약간 아래로 이동
		ground->scale = glm::vec3(100.0f, 0.3f, 100.0f); // 땅을 더 넓게 스케일링
		g_gameWorld.AddObject(std::move(ground));


		// Tino 객체 생성 및 GameWorld에 추가
		// 경로 수정: assets 폴더로 직접 접근
		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj",
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // 전역 포인터에 할당
		tino->position = glm::vec3(-3.0f, -3.0f, 0.0f);  // Ground 위에 배치
		tino->scale = glm::vec3(1.3f, 1.3f, 1.3f);     // 크기 조정 (우선 기본 크기로)
		g_gameWorld.AddObject(std::move(tino_ptr));

		gView = glm::mat4(1.0f);
		gView = glm::lookAt(		//
			glm::vec3(0.0f, 0.0f, 10.0f),  //	EYE
			glm::vec3(0.0f, 0.0f, 0.0f),  // 							AT
			glm::vec3(0.0f, 1.0f, 0.0f)   //				 			UP
		);
	}
	// PLAYING 상태에서만 ObstacleSpawner 추가
	else if (scene == GameState::PLAYING) {

		g_gameWorld.Clear(); // 이전 게임 객체들 제거
		// Ground 객체 생성 및 GameWorld에 추가
		auto ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f });
		ground->scale = glm::vec3(100.0f, 0.3f, 1.3f); // 땅을 더 넓게 스케일링

		g_gameWorld.AddObject(std::move(ground));

		// Ground 객체 생성 및 GameWorld에 추가
		auto ground2 = std::make_unique<Ground>(1, RGBA{ 175 / 255., 145 / 255., 100 / 255., 1.0f });
		ground2->position.y = -4.0f; // 땅을 약간 아래로 이동
		ground2->scale = glm::vec3(100.0f, 0.3f, 100.0f); // 땅을 더 넓게 스케일링
		g_gameWorld.AddObject(std::move(ground2));
		
		// Tino 객체 생성 및 GameWorld에 추가
		// 경로 수정: assets 폴더로 직접 접근
		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj", 
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // 전역 포인터에 할당
		tino->position = glm::vec3(0.0f, 0.0f, 0.0f);  // Ground 위에 배치
		tino->scale = glm::vec3(1.0f, 1.0f, 1.0f);     // 크기 조정 (우선 기본 크기로)

		g_gameWorld.AddObject(std::move(tino_ptr));

		std::cout << "PLAYING 모드 시작 - ObstacleSpawner 추가" << std::endl;
		auto spawner = std::make_unique<ObstacleSpawner>();
		g_gameWorld.AddObject(std::move(spawner));

		auto score = std::make_unique<ScoreDisplay>(
			-0.95f,
			0.95f,
			0.05f,
			0.1f,
			"assets/score_text.png"
		);
		scoreDisplay = score.get();
		scoreDisplay->SetScore(0);
		g_gameWorld.AddObject(std::move(score));

		gView = glm::mat4(1.0f);
		gView = glm::lookAt(		//카메라 외부파라미터
			glm::vec3(-10.0f, 6.0f, 7.0f),  // 카메라 위치 (x, y, z축이 모두 보이는 위치)	EYE
			glm::vec3(0.0f, 2.0f, -3.0f),  // 바라보는 지점 (원점) 							AT
			glm::vec3(0.0f, 1.0f, 0.0f)   // 위쪽 방향 벡터 					 			UP
		);
	}
}

//--- 출력 콜백함수
GLvoid drawScene()
{
	glClearColor(0.7f, 0.95f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//GL_DEPTH_BUFFER_BIT 깊이에 따른 은면제거


	//--- 렌더링 파이프라인에 세이더 불러우기
	glUseProgram(shaderProgramID);

	//gView = glm::mat4(1.0f);
	//gView = glm::lookAt(		//카메라 외부파라미터
	//	glm::vec3(-10.0f, 6.0f, 7.0f),  // 카메라 위치 (x, y, z축이 모두 보이는 위치)	EYE
	//	glm::vec3(0.0f, 2.0f, -3.0f),  // 바라보는 지점 (원점) 							AT
	//	glm::vec3(0.0f, 1.0f, 0.0f)   // 위쪽 방향 벡터 					 			UP
	//);

	// 좌표축 그리기 (변환 행렬 적용)
	DrawAxes(gProjection, gView, uMVP_loc);

	// GameWorld를 통해 모든 객체 렌더링
	g_gameWorld.DrawAll(gProjection, gView, uMVP_loc);

	// 게임 상태 표시 (콘솔)
	static int frameCount = 0;
	frameCount++;
	if (frameCount % 300 == 0) { // 5초마다 출력
		std::cout << "현재 게임 상태: ";
		switch (scene) {
		case GameState::TITLE: std::cout << "TITLE"; break;
		case GameState::LOBBY: std::cout << "LOBBY"; break;
		case GameState::PLAYING: std::cout << "PLAYING"; break;
		case GameState::GAME_OVER: std::cout << "GAME_OVER"; break;
		}
		std::cout << ", 활성 객체 수: " << g_gameWorld.GetActiveObjectCount() << std::endl;
	}

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
	const float deltaTime = 0.016f;

	if (scene == GameState::PLAYING) {	// 게임 스코어 증가
		gameScore += 1;
		if (scoreDisplay) {
			scoreDisplay->SetScore(gameScore);
		}
	}

	// GameWorld를 통해 모든 객체 업데이트 (ObstacleSpawner 포함)
	g_gameWorld.UpdateAll();

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1); // 약 60FPS로 타이머 시작
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	const float deltaTime = 0.016f; // 약 60FPS 기준

	switch (key) {
	case ' ':	// 점프 (추후 구현)
		if (scene == GameState::PLAYING)
			tino->StateChange(State::JUMPING);
		break;
	case '\r': 
	case '\n':		// 엔터 누르면 시작
		if (scene == GameState::TITLE) {
			scene = GameState::PLAYING;
			std::cout << "게임 시작" << std::endl;
			InitGameObjects();		// 게임 객체 초기화
		}
		
		break;
	case 'q':
	case 'Q':
		exit(0);
		break;

	default: break;
	}

	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		if (scene == GameState::PLAYING) {
			tino->StateChange(State::SLIDING);
		}
		glutPostRedisplay();
	}
}