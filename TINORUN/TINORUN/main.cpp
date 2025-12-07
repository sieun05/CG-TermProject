#include "Header.h"
#include "WindowToNDC.h"
#include "shader_func.h"
#include "game_state.h"
#include "game_world.h"
#include "ground.h"

#include "tino.h"  // Tino Çì´õ Ãß°¡
#include "obstacle.h" // Àå¾Ö¹° Çì´õ Ãß°¡ 
#include "Images.h"	// ¹öÆ° Çì´õ Ãß°¡
#include "ScoreDisplay.h"	// Á¡¼ö Çì´õ Ãß°¡
#include "Light.h"	// Á¶¸í Çì´õ Ãß°¡


#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"	// ?¬ìš´???¤ë” ì¶”ê?
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"	// png ?¬ìš©

void InitBuffer();
void InitGameObjects();

GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
//GLvoid SpecialKeyDown(int key, int x, int y);
//GLvoid SpecialKeyUp(int key, int x, int y);

glm::mat4 gProjection(1.0f);
glm::mat4 gView(1.0f);
glm::mat4 gModel(1.0f);
GLint uMVP_loc = -1;


// º¯È¯ Çà·Ä °ü·Ã uniform º¯¼ö Á¤ÀÇ
GLint uModel_loc = -1;
GLint uView_loc = -1;
GLint uProjection_loc = -1;

// ÅØ½ºÃ³ °ü·Ã uniform º¯¼ö Á¤ÀÇ
GLint uUseTexture_loc = -1;
GLint uTextureSampler_loc = -1;

// Á¶¸í °ü·Ã uniform º¯¼ö Á¤ÀÇ
GLint uUseLighting_loc = -1;

// °ÔÀÓ »óÅÂ °ü·Ã º¯¼ö Á¤ÀÇ (sceneÀº game_world.cpp¿¡¼­ Á¤ÀÇµÊ)
bool gameover_flag222 = false;

Tino* tino = nullptr;
ScoreDisplay* scoreDisplay = nullptr;
int gameScore = 0;

// ?¬ìš´???„ì—­ ë³€??
ma_engine engine;
ma_result result;

ma_sound sounds[4];


float sky_x = 0.0f;
bool timer = true;


// Á¶¸í ½Ã°£ º¯¼ö (ÇÏ·ç ÁÖ±â ½Ã¹Ä·¹ÀÌ¼Ç¿ë)

//--- ¸ŞÀÎ ÇÔ¼ö

void main(int argc, char** argv)
//--- ?ˆë„?°ì¶œ?¥í•˜ê³ ì½œë°±í•¨?˜ì„¤??
{
	GLint width, height;

	width = 1600;
	height = 900;
	//--- ?ˆë„?°ìƒ?±í•˜ê¸?
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);		//GLUT_DEPTH ê¹Šì´???°ë¥¸ ?€ë©´ì œê±?
	glutInitWindowPosition(100, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("TINO RUN");

	//--- GLEW ì´ˆê¸°?”í•˜ê¸?
	glewExperimental = GL_TRUE;
	glewInit();

	//--- ?¸ì´?”ì½?´ì??œì„¸?´ë”?„ë¡œê·¸ë¨ë§Œë“¤ê¸? ?¬ìš©?ì •?˜í•¨?˜í˜¸ì¶?
	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();
	AfterMakeShaders();	//?°ì´?”ì—??uniform ë³€???„ì¹˜ ?»ê¸°

	// Á¶¸í ½Ã½ºÅÛ ÃÊ±âÈ­
	g_lightManager.InitializeUniforms(shaderProgramID);
	g_lightManager.SetupSunlight(); // ÀÚ¿¬½º·¯¿î ÅÂ¾ç±¤ ¼³Á¤
	g_lightManager.EnableLighting(true);

	glutReshapeFunc(Reshape);
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	//glutSpecialFunc(SpecialKeyDown);    // ?”ì‚´?????¹ìˆ˜???Œë¦¼ ì²˜ë¦¬
	//glutSpecialUpFunc(SpecialKeyUp);

	glutTimerFunc(16, Timer, 1); // ¾à 60FPS·Î Å¸ÀÌ¸Ó ½ÃÀÛ


	glutTimerFunc(16, Timer, 1); // ??60FPSë¡??€?´ë¨¸ ?œì‘

	InitBuffer();
	InitGameObjects();		// ê²Œì„ ê°ì²´ ì´ˆê¸°??
	
	// ê²Œì„ ?íƒœë¥?PLAYING?¼ë¡œ ?¤ì • (?ŒìŠ¤?¸ìš©)
	scene = GameState::TITLE;

	// ?¬ìš´??ì´ˆê¸°??
	result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) {
		std::cout << "Failed to initialize audio engine." << std::endl;
		return;
	}
	// ?¬ìš´??ì´ˆê¸°??
	ma_sound_init_from_file(&engine, "assets/jump_sound.mp3", 0, NULL, NULL, &sounds[0]);
	ma_sound_init_from_file(&engine, "assets/gameover_sound.mp3", 0, NULL, NULL, &sounds[1]);

	ma_sound_init_from_file(&engine, "assets/background.mp3", 0, NULL, NULL, &sounds[2]);
	ma_sound_init_from_file(&engine, "assets/slide1.mp3", 0, NULL, NULL, &sounds[3]);

	ma_sound_set_looping(&sounds[2], MA_TRUE);  // ë°°ê²½?Œì•… ë£¨í”„ ?¤ì •
	glutMainLoop();
}

void InitBuffer()
{
	glEnable(GL_DEPTH_TEST); // ê¹Šì´ë²„í¼ ?œì„±??
	glEnable(GL_CULL_FACE);

	//RectInit();<< ?°ë¡œ ì´ˆê¸°???¨ìˆ˜ ë§Œë“¤?´ì„œ ?¸ì¶œë§??˜ê¸° 
	GroundInit();
}

void InitGameObjects()
{
	if (scene == GameState::TITLE) {
		g_gameWorld.Clear(); // ?´ì „ ê²Œì„ ê°ì²´???œê±°

		// ?¬ì¸??ì´ˆê¸°??
		scoreDisplay = nullptr;
		tino = nullptr;

		/*auto press_enter = std::move(std::make_unique<Images>(0.5f, -1.0f, 0.8f, 0.3f, "assets/Press_Enter.png"));
		g_gameWorld.AddObject(std::move(press_enter));*/

		auto start_tex = std::move(std::make_unique<Images>(0.0f, 0.0f, 0.0f, 2.0f, 2.0f, "assets/start_texture.png"));
		g_gameWorld.AddObject(std::move(start_tex));

		// Ground ê°ì²´ ?ì„± ë°?GameWorld??ì¶”ê?
		auto ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f });
		ground->position.y = -4.0f; // ?…ì„ ?½ê°„ ?„ë˜ë¡??´ë™
		ground->scale = glm::vec3(100.0f, 0.3f, 100.0f); // ?…ì„ ???“ê²Œ ?¤ì??¼ë§
		g_gameWorld.AddObject(std::move(ground));


		// Tino ê°ì²´ ?ì„± ë°?GameWorld??ì¶”ê?
		// ê²½ë¡œ ?˜ì •: assets ?´ë”ë¡?ì§ì ‘ ?‘ê·¼
		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj",
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // ?„ì—­ ?¬ì¸?°ì— ? ë‹¹
		tino->position = glm::vec3(3.0f, -3.0f, 0.0f);  // Ground ?„ì— ë°°ì¹˜
		tino->scale = glm::vec3(1.3f, 1.3f, 1.3f);     // ?¬ê¸° ì¡°ì • (?°ì„  ê¸°ë³¸ ?¬ê¸°ë¡?
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
		ma_sound_stop(&sounds[2]); // ë°°ê²½?Œì•… ?•ì?
	}
	// PLAYING ?íƒœ?ì„œë§?ObstacleSpawner ì¶”ê?
	else if (scene == GameState::PLAYING) {

		g_gameWorld.Clear(); // ?´ì „ ê²Œì„ ê°ì²´???œê±°

		// ?˜ëŠ˜ ë°°ê²½
		auto sky = std::move(std::make_unique<Images>(0.0f, 0.7f, -1.0f, 2.0f, 0.6f, "assets/sky_2.png"));
		g_gameWorld.AddObject(std::move(sky));

		// Ground ê°ì²´ ?ì„± ë°?GameWorld??ì¶”ê?
		auto ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f }, "assets/sand_texture4.png");
		ground->scale = glm::vec3(100.0f, 0.3f, 1.3f); // ?…ì„ ???“ê²Œ ?¤ì??¼ë§
		g_gameWorld.AddObject(std::move(ground));

		// Ground ê°ì²´ ?ì„± ë°?GameWorld??ì¶”ê?
		auto back_ground = std::make_unique<Ground>(1, RGBA{ 231 / 255., 217 / 255., 176 / 255., 1.0f }, "assets/sand_texture4.png");
		back_ground->scale = glm::vec3(100.0f, 0.3f, 1.3f); // ?…ì„ ???“ê²Œ ?¤ì??¼ë§
		back_ground->position.z = -10.0f; // ?¤ìª½??ë°°ì¹˜
		g_gameWorld.AddObject(std::move(back_ground));


		// Ground ê°ì²´ ?ì„± ë°?GameWorld??ì¶”ê?
		auto ground2 = std::make_unique<Ground>(1, RGBA{ 175 / 255., 145 / 255., 100 / 255., 1.0f } , "assets/ground_texture2.png");
		ground2->position.y = -4.0f; // ?…ì„ ?½ê°„ ?„ë˜ë¡??´ë™
		ground2->scale = glm::vec3(100.0f, 0.3f, 100.0f); // ?…ì„ ???“ê²Œ ?¤ì??¼ë§
		g_gameWorld.AddObject(std::move(ground2));
		

		// Tino ê°ì²´ ?ì„± ë°?GameWorld??ì¶”ê?
		// ê²½ë¡œ ?˜ì •: assets ?´ë”ë¡?ì§ì ‘ ?‘ê·¼
		auto tino_ptr = std::make_unique<Tino>("assets/Tino.obj", "assets/Tino_jump.obj", 
			"assets/Tino_down.obj", "assets/Tino_base.png");
		tino = tino_ptr.get(); // ?„ì—­ ?¬ì¸?°ì— ? ë‹¹
		tino->position = glm::vec3(0.0f, 0.5f, 0.0f);  // Ground ?„ì— ë°°ì¹˜
		tino->scale = glm::vec3(1.0f, 1.0f, 1.0f);     // ?¬ê¸° ì¡°ì • (?°ì„  ê¸°ë³¸ ?¬ê¸°ë¡?

		g_gameWorld.AddObject(std::move(tino_ptr));

		std::cout << "PLAYING ëª¨ë“œ ?œì‘ - ObstacleSpawner ì¶”ê?" << std::endl;
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
		gView = glm::lookAt(		//ì¹´ë©”???¸ë??Œë¼ë¯¸í„°
			glm::vec3(-12.0f, 7.0f, 10.0f),  // ì¹´ë©”???„ì¹˜ (x, y, zì¶•ì´ ëª¨ë‘ ë³´ì´???„ì¹˜)	EYE
			glm::vec3(0.0f, 2.0f, -3.0f),  // ë°”ë¼ë³´ëŠ” ì§€??(?ì ) 							AT
			glm::vec3(0.0f, 1.0f, 0.0f)   // ?„ìª½ ë°©í–¥ ë²¡í„° 					 			UP
		);

		// ë°°ê²½?Œì•… ?¬ìƒ (ì²˜ìŒë¶€???œì‘)
		ma_sound_stop(&sounds[2]);  // ?¹ì‹œ ?¬ìƒ ì¤‘ì´ë©??•ì?
		ma_sound_seek_to_pcm_frame(&sounds[2], 0);  // ì²˜ìŒ?¼ë¡œ
		ma_sound_start(&sounds[2]);  // ?¬ìƒ ?œì‘

		if (!timer) {
			timer = true;
			glutTimerFunc(16, Timer, 1);
		}
	}
	else if (scene == GameState::GAME_OVER) {
		g_gameWorld.Clear(); // ?´ì „ ê²Œì„ ê°ì²´???œê±°

		if(timer) timer = false;

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
		tino = tino_ptr.get(); // ?„ì—­ ?¬ì¸?°ì— ? ë‹¹
		tino->position = glm::vec3(-0.1f, 0.0f, 0.0f);  // Ground ?„ì— ë°°ì¹˜
		tino->scale = glm::vec3(1.0f, 1.0f, 1.0f);     // ?¬ê¸° ì¡°ì • (?°ì„  ê¸°ë³¸ ?¬ê¸°ë¡?
		//tino->rotation = glm::vec3(0.0f, 90.0f, 0.0f); // ?•íŒ ?íƒœë¡??Œì „
		tino->StateChange(State::JUMPING);

		// ? ì¸??- ?¼ìª½
		auto cactus = std::make_unique<Cactus>("assets/obstacle1.obj", "assets/obstacle1_base.bmp");
		cactus->position = glm::vec3(-3.0f, -3.0f, 0.0f);
		cactus->scale = glm::vec3(0.2f, 0.2f, 0.2f);
		cactus->rotation = glm::vec3(0.0f, 30.0f, 20.0f);
		cactus->SetSpeed(0.0f);  // ?€ì§ì´ì§€ ?Šê²Œ
		g_gameWorld.AddObject(std::move(cactus));

		// ?˜ë¬´ - ?¤ë¥¸ìª?
		auto tree = std::make_unique<Tree>("assets/obstacle2.obj", "assets/obstacle2_base.bmp");
		tree->position = glm::vec3(8.0f, 2.0f, -2.0f);
		tree->scale = glm::vec3(0.3f, 0.3f, 0.3f);
		tree->rotation = glm::vec3(20.0f, -45.0f, 45.0f);
		tree->SetSpeed(0.0f);
		g_gameWorld.AddObject(std::move(tree));

		// ë²„ì„¯ - ì¤‘ì•™ ?¤ìª½
		auto mushroom = std::make_unique<Mushroom>("assets/obstacle3.obj", "assets/obstacle3_base.bmp");
		mushroom->position = glm::vec3(-8.0f, 2.0f, -5.0f);
		mushroom->scale = glm::vec3(1.2f, 1.2f, 1.2f);
		mushroom->rotation = glm::vec3(10.0f, 0.0f, -10.0f);
		mushroom->SetSpeed(0.0f);
		g_gameWorld.AddObject(std::move(mushroom));

		// ??- ê³µì¤‘
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

		// ë°°ê²½?Œì•… ?•ì?
		ma_sound_stop(&sounds[2]);

		// ê²Œì„?¤ë²„ ?¬ìš´???¬ìƒ
		ma_sound_start(&sounds[1]);
		ma_sound_seek_to_pcm_frame(&sounds[1], 0);
	}
}

//--- ì¶œë ¥ ì½œë°±?¨ìˆ˜
GLvoid drawScene()
{
	glClearColor(0.7f, 0.95f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//GL_DEPTH_BUFFER_BIT ê¹Šì´???°ë¥¸ ?€ë©´ì œê±?


	//--- ?Œë”ë§??Œì´?„ë¼?¸ì— ?¸ì´??ë¶ˆëŸ¬?°ê¸°
	glUseProgram(shaderProgramID);


	// GameWorldë¥??µí•´ ëª¨ë“  ê°ì²´ ?Œë”ë§?
	g_gameWorld.DrawAll(gProjection, gView, uMVP_loc);

	// ê²Œì„ ?íƒœ ?œì‹œ (ì½˜ì†”)
	static int frameCount = 0;
	frameCount++;
	if (frameCount % 300 == 0) { // 5ì´ˆë§ˆ??ì¶œë ¥
		std::cout << "?„ì¬ ê²Œì„ ?íƒœ: ";
		switch (scene) {
		case GameState::TITLE: std::cout << "TITLE"; break;
		case GameState::PLAYING: std::cout << "PLAYING"; break;
		case GameState::GAME_OVER: std::cout << "GAME_OVER"; break;
		}
		std::cout << ", ?œì„± ê°ì²´ ?? " << g_gameWorld.GetActiveObjectCount() << std::endl;
	}

	glutSwapBuffers();
}

//--- ?¤ì‹œê·¸ë¦¬ê¸°ì½œë°±í•¨??
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	glUseProgram(shaderProgramID);

	float aspect = (h == 0) ? 1 : (float)w / (float)h;

	//?ê·¼ ?¬ì˜ ?¬ìš© (3D ?¨ê³¼ë¥?????ë³´ì—¬ì¤?
	gProjection = glm::perspective(
		glm::radians(45.0f),  // ?œì•¼ê°?45??fovy
		aspect,               // ì¢…íš¡ë¹?		aspect
		0.1f,                 // ê·¼í‰ë©?		-n
		100.0f                // ?í‰ë©?		-f
	);

	glEnable(GL_DEPTH_TEST); // ê¹Šì´ë²„í¼ ?œì„±??
	glUseProgram(0);
}

GLvoid Timer(int value)
{
	if (!timer) return;

	const float deltaTime = 0.016f;

	if (scene == GameState::PLAYING) {	// °ÔÀÓ ½ºÄÚ¾î Áõ°¡
		gameScore += 1;
		if (scoreDisplay) {
			scoreDisplay->SetScore(gameScore);
		}
	}
	if (scene == GameState::PLAYING and gameover_flag222) {
		scene = GameState::GAME_OVER;
		InitGameObjects();
	}

	// Á¶¸í ¾÷µ¥ÀÌÆ® Á¦°Å - °íÁ¤µÈ ÅÂ¾çºû À¯Áö
	// ÃÊ±âÈ­ ½Ã¿¡¸¸ ¼³Á¤µÇ°í ÀÌÈÄ º¯°æÇÏÁö ¾ÊÀ½
	
	// GameWorld¸¦ ÅëÇØ ¸ğµç °´Ã¼ ¾÷µ¥ÀÌÆ® (ObstacleSpawner Æ÷ÇÔ)
	g_gameWorld.UpdateAll();

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1); // ¾à 60FPS·Î Å¸ÀÌ¸Ó ½ÃÀÛ

}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	const float deltaTime = 0.016f; // ??60FPS ê¸°ì?

	switch (key) {
	case ' ':	// ?í”„ (ì¶”í›„ êµ¬í˜„)
		if (scene == GameState::PLAYING) {
			tino->StateChange(State::JUMPING);
			
			ma_sound_start(&sounds[0]);	// ?í”„ ?¬ìš´??
			ma_sound_seek_to_pcm_frame(&sounds[0], 0);
		}
		break;
	case '\r': 
	case '\n':		// ?”í„° ?„ë¥´ë©??œì‘
		if (scene == GameState::TITLE) {
			scene = GameState::PLAYING;
			std::cout << "ê²Œì„ ?œì‘" << std::endl;
			InitGameObjects();		// ê²Œì„ ê°ì²´ ì´ˆê¸°??
		}
		if(scene == GameState::GAME_OVER) {
			scene = GameState::TITLE;
			gameover_flag222 = false;
			std::cout << "?€?´í? ?”ë©´?¼ë¡œ ?´ë™" << std::endl;
			InitGameObjects();
		}
		break;
	case 27:		
		if (scene == GameState::PLAYING) {
			// ESC ?„ë¥´ë©??€?´í?ë¡?
			scene = GameState::TITLE;
			std::cout << "?€?´í? ?”ë©´?¼ë¡œ ?´ë™" << std::endl;
		}
		else if (scene == GameState::TITLE || scene == GameState::GAME_OVER) {
			// ?€?´í??ì„œ ?„ë¥´ë©?ê²Œì„ì¢…ë£Œ
			std::cout << "ê²Œì„ ì¢…ë£Œ" << std::endl;
			ma_engine_uninit(&engine); // ?´ì „???¬ìƒ ì¤‘ì´???¬ìš´???•ë¦¬
			exit(0);
		}
		InitGameObjects();
		break;
	case 'g':	// ?•ì¸??
		scene = GameState::GAME_OVER;
		InitGameObjects();
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