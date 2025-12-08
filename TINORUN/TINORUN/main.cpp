#include "Header.h"
#include "WindowToNDC.h"
#include "shader_func.h"
#include "game_state.h"
#include "game_world.h"
#include "ground.h"

#include "tino.h"  // Tino ��� �߰�
#include "obstacle.h" // ��ֹ� ��� �߰� 
#include "Images.h"	// ��ư ��� �߰�
#include "ScoreDisplay.h"	// ���� ��� �߰�
#include "Light.h"	// ���� ��� �߰�

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"	
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"	
#include <Windows.h>

//
void InitBuffer();
void InitGameObjects();
void ShowExistingConsole();

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
//GLvoid SpecialKeyDown(int key, int x, int y);
//GLvoid SpecialKeyUp(int key, int x, int y);

// Shader variables
GLuint shaderProgramID = 0;
GLuint vertexShader = 0;
GLuint fragmentShader = 0;
GLuint shadowVertexShader = 0;
GLuint shadowFragmentShader = 0;

glm::mat4 gProjection(1.0f);
glm::mat4 gView(1.0f);
glm::mat4 gModel(1.0f);
GLint uMVP_loc = -1;

// 텍스처 관련 uniform 변수 선언
GLint uUseTexture_loc = -1;
GLint uTextureSampler_loc = -1;

// Lighting and shadow uniform locations
GLint uModel_loc = -1;
GLint uView_loc = -1;
GLint uProjection_loc = -1;
GLint uLightSpaceMatrix_loc = -1;
GLint uUseLighting_loc = -1;
GLint uUseShadows_loc = -1;
GLint uLightDir_loc = -1;
GLint uLightColor_loc = -1;
GLint uViewPos_loc = -1;
GLint uAmbientStrength_loc = -1;
GLint uSpecularStrength_loc = -1;
GLint uShininess_loc = -1;
GLint uShadowMap_loc = -1;

// Shadow system
GLuint shadowMapFBO = 0;
GLuint shadowMapTexture = 0;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
GLuint shadowShaderProgram = 0;
glm::mat4 lightSpaceMatrix(1.0f);

// ���� ���� ���� ���� ���� (scene�� game_world.cpp���� ���ǵ�)
bool gameover_flag222 = false;

Tino* tino = nullptr;
ScoreDisplay* scoreDisplay = nullptr;
int gameScore = 0;
bool showBoundaryBox = false;  // 바운더리 박스 표시 여부

// ���� ���� ����
ma_engine engine;
ma_result result;

ma_sound sounds[4];

bool timer = false;


// ���� �ð� ���� (�Ϸ� �ֱ� �ùķ��̼ǿ�)
float currentTime = 0.5f; // 0.5 = �������� ����

//--- ���� �Լ�

void main(int argc, char** argv)
//--- ����������ϰ��ݹ��Լ�����
{
	GLint width, height;

	width = 1600;
	height = 900;
	//--- ����������ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);		//GLUT_DEPTH 깊이???�른 ?�면제�?
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("TINO RUN");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();

	//--- ���̴��о�ͼ����̴����α׷������: ����������Լ�ȣ��
	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();
	AfterMakeShaders();	//?�이?�에??uniform 변???�치 ?�기

	// ���� �ý��� �ʱ�ȭ
	g_lightManager.InitializeUniforms(shaderProgramID);
	g_lightManager.SetupSunlight(); // �ڿ������� �¾籤 ����
	g_lightManager.EnableLighting(false);

	glutReshapeFunc(Reshape);
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	//glutSpecialFunc(SpecialKeyDown);    // ?�살?????�수???�림 처리
	//glutSpecialUpFunc(SpecialKeyUp);

	InitBuffer();
	InitGameObjects();		// 게임 객체 초기??

	// ���� ���¸� PLAYING���� ���� (�׽�Ʈ��)
	scene = GameState::TITLE;

	// ���� �ʱ�ȭ
	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
		std::cout << "Failed to initialize audio engine." << std::endl;
		return;
	}
	// ���� �ʱ�ȭ
	ma_sound_init_from_file(&engine, "assets/jump_sound.mp3", 0, NULL, NULL, &sounds[0]);
	ma_sound_init_from_file(&engine, "assets/gameover_sound.mp3", 0, NULL, NULL, &sounds[1]);

	ma_sound_init_from_file(&engine, "assets/background.mp3", 0, NULL, NULL, &sounds[2]);
	ma_sound_init_from_file(&engine, "assets/slide1.mp3", 0, NULL, NULL, &sounds[3]);

	ma_sound_set_looping(&sounds[2], MA_TRUE);  // 배경?�악 루프 ?�정

	ShowWindow(GetConsoleWindow(), SW_HIDE);
	glutMainLoop();
}

void InitBuffer()
{
	glEnable(GL_DEPTH_TEST); // 깊이버퍼 ?�성??
	glEnable(GL_CULL_FACE);

	//RectInit();<< ?�로 초기???�수 만들?�서 ?�출�??�기 
	GroundInit();
}

void InitGameObjects()
{
	if (scene == GameState::TITLE) {
		g_gameWorld.Clear(); // ?�전 게임 객체???�거

		gameScore = 0;

		// ?�인??초기??
		scoreDisplay = nullptr;
		tino = nullptr;

		/*auto press_enter = std::move(std::make_unique<Images>(0.5f, -1.0f, 0.8f, 0.3f, "assets/Press_Enter.png"));
		g_gameWorld.AddObject(std::move(press_enter));*/

		auto start_tex = std::move(std::make_unique<Images>(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, "assets/start_texture.png"));
		g_gameWorld.AddObject(std::move(start_tex));

		// Ground 객체 ?�성 �?GameWorld??추�?
		auto ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f });
		ground->position.y = -4.0f;
		ground->scale = glm::vec3(100.0f, 0.3f, 100.0f);
		g_gameWorld.AddObject(std::move(ground));


		// Tino ��ü ���� �� GameWorld�� �߰�
		// ��� ����: assets ������ ���� ����
		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj",
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // ?�역 ?�인?�에 ?�당
		tino->position = glm::vec3(3.0f, -3.0f, 0.0f);  // Ground ?�에 배치
		tino->scale = glm::vec3(1.3f, 1.3f, 1.3f);     // ?�기 조정 (?�선 기본 ?�기�?
		g_gameWorld.AddObject(std::move(tino_ptr));

		gView = glm::mat4(1.0f);
		gView = glm::lookAt(		//
			glm::vec3(0.0f, 0.0f, 10.0f),  //	EYE
			glm::vec3(0.0f, 0.0f, 0.0f),  // 							AT
			glm::vec3(0.0f, 1.0f, 0.0f)   //				 			UP
		);

		if (!timer) {
			timer = true;
			glutTimerFunc(16, Timer, 1);
		}
		ma_sound_stop(&sounds[2]); // 배경?�악 ?��?
	}
	// PLAYING ?�태?�서�?ObstacleSpawner 추�?
	else if (scene == GameState::PLAYING) {

		g_gameWorld.Clear(); // ?�전 게임 객체???�거

		// ?�늘 배경
		auto sky = std::move(std::make_unique<Images>(0.0f, 0.7f, -1.0f, 2.0f, 0.6f, "assets/sky_2.png"));
		g_gameWorld.AddObject(std::move(sky));

		// Ground 객체 ?�성 �?GameWorld??추�?
		auto ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f }, "assets/sand_texture4.png");
		ground->scale = glm::vec3(100.0f, 0.3f, 1.3f); // ?�을 ???�게 ?��??�링
		g_gameWorld.AddObject(std::move(ground));

		// Ground 객체 ?�성 �?GameWorld??추�?
		auto back_ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f }, "assets/sand_texture4.png");
		back_ground->scale = glm::vec3(100.0f, 0.3f, 1.3f); // ?�을 ???�게 ?��??�링
		back_ground->position.z = -10.0f; // ?�쪽??배치
		g_gameWorld.AddObject(std::move(back_ground));


		// Ground 객체 ?�성 �?GameWorld??추�?
		auto ground2 = std::make_unique<Ground>(1, RGBA{ 175 / 255., 145 / 255., 100 / 255., 1.0f }, "assets/ground_texture2.png");
		ground2->position.y = -4.0f; // ?�을 ?�간 ?�래�??�동
		ground2->scale = glm::vec3(100.0f, 0.3f, 100.0f); // ?�을 ???�게 ?��??�링
		g_gameWorld.AddObject(std::move(ground2));


		// Tino 객체 ?�성 �?GameWorld??추�?
		// 경로 ?�정: assets ?�더�?직접 ?�근
		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj",
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // ?�역 ?�인?�에 ?�당
		tino->position = glm::vec3(0.0f, 0.5f, 0.0f);  // Ground ?�에 배치
		tino->scale = glm::vec3(1.0f, 1.0f, 1.0f);     // ?�기 조정 (?�선 기본 ?�기�?

		g_gameWorld.AddObject(std::move(tino_ptr));

		//std::cout << "PLAYING 모드 ?�작 - ObstacleSpawner 추�?" << std::endl;
		auto spawner = std::make_unique<ObstacleSpawner>();
		spawner->SetbackSpawnFlag(false); 
		g_gameWorld.AddObject(std::move(spawner));

		auto back_spawner = std::make_unique<ObstacleSpawner>();
		back_spawner->SetbackSpawnFlag(true); // 배경용
		g_gameWorld.AddObject(std::move(back_spawner));

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
		gView = glm::lookAt(		//카메???��??�라미터
			glm::vec3(-12.0f, 7.0f, 10.0f),  // 카메???�치 (x, y, z축이 모두 보이???�치)	EYE
			glm::vec3(0.0f, 2.0f, -3.0f),  // 바라보는 지??(?�점) 							AT
			glm::vec3(0.0f, 1.0f, 0.0f)   // ?�쪽 방향 벡터 					 			UP
		);

		// 배경?�악 ?�생 (처음부???�작)
		ma_sound_stop(&sounds[2]);  // ?�시 ?�생 중이�??��?
		ma_sound_seek_to_pcm_frame(&sounds[2], 0);  // 처음?�로
		ma_sound_start(&sounds[2]);  // ?�생 ?�작

		if (!timer) {
			timer = true;
			glutTimerFunc(16, Timer, 1);
		}
	}
	else if (scene == GameState::GAME_OVER) {
		g_gameWorld.Clear(); // ?�전 게임 객체???�거

		if (timer) timer = false;

		auto gameover = std::move(std::make_unique<Images>(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, "assets/gameover.png"));
		g_gameWorld.AddObject(std::move(gameover));

		auto score = std::make_unique<ScoreDisplay>(
			-0.2f,
			0.0f,
			0.1f,
			0.2f,
			"assets/score_text.png"
		);
		scoreDisplay = score.get();
		scoreDisplay->SetScore(gameScore);
		g_gameWorld.AddObject(std::move(score));

		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj",
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // ?�역 ?�인?�에 ?�당
		tino->position = glm::vec3(-0.1f, 0.0f, 0.0f);  // Ground ?�에 배치
		tino->scale = glm::vec3(1.0f, 1.0f, 1.0f);     // ?�기 조정 (?�선 기본 ?�기�?
		//tino->rotation = glm::vec3(0.0f, 90.0f, 0.0f); // ?�힌 ?�태�??�전
		tino->StateChange(State::JUMPING);

		// ?�인??- ?�쪽
		auto cactus = std::make_unique<Cactus>("assets/obstacle1.obj", "assets/obstacle1_base.bmp");
		cactus->position = glm::vec3(-3.0f, -3.0f, 0.0f);
		cactus->scale = glm::vec3(0.2f, 0.2f, 0.2f);
		cactus->rotation = glm::vec3(0.0f, 30.0f, 20.0f);
		cactus->SetSpeed(0.0f);  // ?�직이지 ?�게
		g_gameWorld.AddObject(std::move(cactus));

		// ?�무 - ?�른�?
		auto tree = std::make_unique<Tree>("assets/obstacle2.obj", "assets/obstacle2_base.bmp");
		tree->position = glm::vec3(8.0f, 2.0f, -2.0f);
		tree->scale = glm::vec3(0.3f, 0.3f, 0.3f);
		tree->rotation = glm::vec3(20.0f, -45.0f, 45.0f);
		tree->SetSpeed(0.0f);
		g_gameWorld.AddObject(std::move(tree));

		// 버섯 - 중앙 ?�쪽
		auto mushroom = std::make_unique<Mushroom>("assets/obstacle3.obj", "assets/obstacle3_base.bmp");
		mushroom->position = glm::vec3(-8.0f, 2.0f, -5.0f);
		mushroom->scale = glm::vec3(1.2f, 1.2f, 1.2f);
		mushroom->rotation = glm::vec3(10.0f, 0.0f, -10.0f);
		mushroom->SetSpeed(0.0f);
		g_gameWorld.AddObject(std::move(mushroom));

		// ??- 공중
		auto bird = std::make_unique<Bird>("assets/bird.obj", "assets/bird_base.bmp");
		bird->position = glm::vec3(5.0f, -2.0f, 0.0f);
		bird->scale = glm::vec3(0.8f, 0.8f, 0.8f);
		bird->rotation = glm::vec3(0.0f, -180.0f, 0.0f);
		bird->SetSpeed(0.0f);
		g_gameWorld.AddObject(std::move(bird));

		gView = glm::mat4(1.0f);
		gView = glm::lookAt(		//
			glm::vec3(0.0f, 0.0f, 10.0f),  //	EYE
			glm::vec3(0.0f, 0.0f, 0.0f),  // 							AT
			glm::vec3(0.0f, 1.0f, 0.0f)   //				 			UP
		);

		// 배경?�악 ?��?
		ma_sound_stop(&sounds[2]);

		// 게임?�버 ?�운???�생
		ma_sound_start(&sounds[1]);
		ma_sound_seek_to_pcm_frame(&sounds[1], 0);

		gameScore = 0;
	}
}

//--- 출력 콜백?�수
GLvoid drawScene()
{
	glClearColor(0.7f, 0.95f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//GL_DEPTH_BUFFER_BIT 깊이???�른 ?�면제�?


	//--- ?�더�??�이?�라?�에 ?�이??불러?�기
	glUseProgram(shaderProgramID);

	// GameWorld�� ���� ��� ��ü ������
	g_gameWorld.DrawAll(gProjection, gView, uMVP_loc);

	// 게임 ?�태 ?�시 (콘솔)
	//static int frameCount = 0;
	//frameCount++;
	//if (frameCount % 300 == 0) { // 5초마??출력
	//	//std::cout << "?�재 게임 ?�태: ";
	//	switch (scene) {
	//	case GameState::TITLE: //std::cout << "TITLE"; break;
	//	case GameState::PLAYING: //std::cout << "PLAYING"; break;
	//	case GameState::GAME_OVER:// std::cout << "GAME_OVER"; break;
	//	}
	//	//std::cout << ", ?�성 객체 ?? " << g_gameWorld.GetActiveObjectCount() << std::endl;
	//}

	glutSwapBuffers();
}

//--- ?�시그리기콜백함??
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	glUseProgram(shaderProgramID);

	float aspect = (h == 0) ? 1 : (float)w / (float)h;

	//?�근 ?�영 ?�용 (3D ?�과�?????보여�?
	gProjection = glm::perspective(
		glm::radians(45.0f),  // ?�야�?45??fovy
		aspect,               // 종횡�?		aspect
		0.1f,                 // 근평�?		-n
		100.0f                // ?�평�?		-f
	);

	glEnable(GL_DEPTH_TEST); // 깊이버퍼 ?�성??
	glUseProgram(0);
}

GLvoid Timer(int value)
{
	if (!timer) return;

	const float deltaTime = 0.016f;

	if (scene == GameState::PLAYING) {
		gameScore += 1;
		if (scoreDisplay) {
			scoreDisplay->SetScore(gameScore);
		}
	}
	if (scene == GameState::PLAYING and gameover_flag222) {
		scene = GameState::GAME_OVER;

		ShowExistingConsole();
		std::cout << "Input your name: ";
		string name;
		std::cin >> name;
		PostScoreToServer(name.c_str(), gameScore);
		//PostScoreToServer("Player", gameScore);
		ShowWindow(GetConsoleWindow(), SW_HIDE);

		InitGameObjects();
	}

	glUseProgram(shaderProgramID);
	g_lightManager.UpdateSunlight(0.0f);
	g_lightManager.SendLightsToShader();

	glUseProgram(0);

	// GameWorld�� ���� ��� ��ü ������Ʈ (ObstacleSpawner ����)
	// GameWorld�??�해 모든 객체 ?�데?�트 (ObstacleSpawner ?�함)
	g_gameWorld.UpdateAll();

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1); // ??60FPS�??�?�머 ?�작

}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	const float deltaTime = 0.016f; // ??60FPS 기�?

	switch (key) {
	case ' ':	// ?�프 (추후 구현)
		if (scene == GameState::PLAYING) {
			tino->StateChange(State::JUMPING);

			ma_sound_start(&sounds[0]);	// ?�프 ?�운??
			ma_sound_seek_to_pcm_frame(&sounds[0], 0);
		}
		break;
	case '\r':
	case '\n':		// ?�터 ?�르�??�작
		if (scene == GameState::TITLE) {
			scene = GameState::PLAYING;
			//std::cout << "게임 ?�작" << std::endl;
			InitGameObjects();		// 게임 객체 초기??
		}
		if (scene == GameState::GAME_OVER) {
			scene = GameState::TITLE;
			gameover_flag222 = false;
			//std::cout << "?�?��? ?�면?�로 ?�동" << std::endl;
			InitGameObjects();
		}
		break;
	case 27:
		if (scene == GameState::PLAYING) {
			// ESC ?�르�??�?��?�?
			scene = GameState::TITLE;
			//std::cout << "?�?��? ?�면?�로 ?�동" << std::endl;
		}
		else if (scene == GameState::TITLE || scene == GameState::GAME_OVER) {
			// ?�?��??�서 ?�르�?게임종료
			//std::cout << "게임 종료" << std::endl;
			ma_engine_uninit(&engine); // ?�전???�생 중이???�운???�리
			exit(0);
		}
		InitGameObjects();
		break;
	case 'g':	
	case 'G':	// 강제 게임 오버 (디버그용)
		scene = GameState::GAME_OVER;
		InitGameObjects();
		break;
	case 'b':	// 바운더리 박스 토글
	case 'B':
		showBoundaryBox = !showBoundaryBox;
		//std::cout << "See BoundaryBox: " << (showBoundaryBox ? "ON" : "OFF") << std::endl;
		break;
	case 'h':
	case 'H':
		tino->SetInvincibility();
		//std::cout << "Invincibility Toggle" << std::endl;
		break;
	case '+':	// 점수 증감(속도 확인용)
	case '=':
		gameScore += 1000;
		break;
	case '-':
	case '_':
		if (gameScore >= 1000)
			gameScore -= 1000;
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
			ma_sound_start(&sounds[3]);
			ma_sound_seek_to_pcm_frame(&sounds[3], 0);
		}
		glutPostRedisplay();
	}
}

void ShowExistingConsole() {
	// 1. 현재 프로세스의 콘솔 창 핸들을 가져옵니다.
	// GetConsoleWindow() 함수는 현재 프로세스에 연결된 콘솔 창의 핸들(HWND)을 반환합니다.
	HWND consoleWindow = GetConsoleWindow();

	if (consoleWindow) {
		// 2. 창을 보이게 설정합니다. (창이 숨겨져 있을 경우)
		// SW_SHOW: 창을 일반적인 상태로 표시합니다. (이전 위치와 크기를 유지)
		ShowWindow(consoleWindow, SW_SHOW);

		// 3. 창을 최상위로 가져와 포커스를 줍니다.
		// SetForegroundWindow: 창을 전경으로 가져와 사용자에게 포커스를 줍니다.
		SetForegroundWindow(consoleWindow);

		// (선택적) 창의 위치나 크기를 조정할 수도 있습니다
	}
}