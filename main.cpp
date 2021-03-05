#include "common.h"
#include "Image.h"
#include "Player.h"

#define GLFW_DLL
#include <cstring>
#include <vector>
#include <unistd.h>

constexpr GLsizei WINDOW_WIDTH = 1440, WINDOW_HEIGHT = 800;

struct InputState
{
  bool keys[1024]{}; //массив состояний кнопок - нажата/не нажата
  GLfloat lastX = 400, lastY = 300; //исходное положение мыши
  bool firstMouse = true;
  bool captureMouse         = true;  // Мышка захвачена нашим приложением или нет?
  bool capturedMouseJustNow = false;
} static Input;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
  case GLFW_KEY_1:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    break;
  case GLFW_KEY_2:
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    break;
	default:
		if (action == GLFW_PRESS)
      Input.keys[key] = true;
		else if (action == GLFW_RELEASE)
      Input.keys[key] = false;
	}
}

void processPlayerMovement(Player &player)
{
  if (Input.keys[GLFW_KEY_W])
    player.ProcessInput(MovementDir::UP);
  else if (Input.keys[GLFW_KEY_S])
    player.ProcessInput(MovementDir::DOWN);
  else if (Input.keys[GLFW_KEY_A])
    player.ProcessInput(MovementDir::LEFT);
  else if (Input.keys[GLFW_KEY_D])
    player.ProcessInput(MovementDir::RIGHT);
  else 
  {
    player.ProcessInput(MovementDir::BLANK);
  }
  if (Input.keys[GLFW_KEY_SPACE])
  {
    Input.keys[GLFW_KEY_SPACE] = false;
    player.getLastAction() = MovementDir::Q;
  }
  else if (Input.keys[GLFW_KEY_E])
  {
    Input.keys[GLFW_KEY_E] = false;
    player.getLastAction() = MovementDir::E;
  }
}

void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    Input.captureMouse = !Input.captureMouse;

  if (Input.captureMouse)
  {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    Input.capturedMouseJustNow = true;
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
  if (Input.firstMouse)
  {
    Input.lastX = float(xpos);
    Input.lastY = float(ypos);
    Input.firstMouse = false;
  }

  GLfloat xoffset = float(xpos) - Input.lastX;
  GLfloat yoffset = Input.lastY - float(ypos);

  Input.lastX = float(xpos);
  Input.lastY = float(ypos);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
  // ...
}

int initGL()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  std::cout << "Controls: "<< std::endl;
  std::cout << "press right mouse button to capture/release mouse cursor  "<< std::endl;
  std::cout << "W, A, S, D - movement  "<< std::endl;
  std::cout << "press ESC to exit" << std::endl;

	return 0;
}

void Initialize(Game & game);

int main(int argc, char** argv)
{
	if(!glfwInit())
    return -1;

//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow*  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "task1 base project", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window); 

	glfwSetKeyCallback        (window, OnKeyboardPressed);  
	glfwSetCursorPosCallback  (window, OnMouseMove); 
  glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback     (window, OnMouseScroll);

	if(initGL() != 0) 
		return -1;
	
  //Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

  std::vector<Room> room_vector;

	Player player;
  //Game game("../resources/TileA4.png","../resources/1.txt");

  player.get_hero().Width() = 24;
  player.get_hero().Height() = 32;
  Image hero_top("../resources/hero_top.png");
  Image hero_down("../resources/hero_down.png");
  Image hero_stay("../resources/hero_stay.png");
  Image hero_left("../resources/hero_left.png");
  Image hero_right("../resources/hero_right.png");
  player.get_hero_top() = hero_move{.direct = &hero_top, .x=0, .size=6, .last_time = (GLfloat)glfwGetTime()};
  player.get_hero_right() = hero_move{.direct = &hero_right, .x=0, .size=6, .last_time = (GLfloat)glfwGetTime()};
  player.get_hero_left() = hero_move{.direct = &hero_left, .x=0, .size=6, .last_time = (GLfloat)glfwGetTime()};
  player.get_hero_down() = hero_move{.direct = &hero_down, .x=0, .size=6, .last_time = (GLfloat)glfwGetTime()};
  player.get_hero_stay() = hero_move{.direct = &hero_stay, .x=0, .size=9, .last_time = (GLfloat)glfwGetTime()};

  
  Game game;
  Initialize(game);

	Image screenBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, 4);

  game.createRoomVector();
  player.getX() = game.getCurRoom().getPlayerX();
  player.getY() = game.getCurRoom().getPlayerY();
  
  screenBuffer.PutImage(0,0,game.getCurRoom().getImage());


  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);  GL_CHECK_ERRORS;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); GL_CHECK_ERRORS;

  //game loop
  int i=0;
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

    glfwPollEvents(); // обработка события из очереди
      

    processPlayerMovement(player);
    
    player.Draw(screenBuffer, game, window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS; //очищает указанные буферы
    glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;
    //прорисовывает картинку
		glfwSwapBuffers(window);

    if (player.getStatus())
    {
      screenBuffer.PutImage(WINDOW_WIDTH/2 - game.get_tile(LOST).Width()/2,
        WINDOW_HEIGHT/2 - game.get_tile(LOST).Height()/2,game.get_tile(LOST),true);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS; //очищает указанные буферы
      glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;
      for (int i=0;i<256;i++)
        glfwSwapBuffers(window);
      break;
    }

    if (player.getWinStatus())
    {
      screenBuffer.PutImage(WINDOW_WIDTH/2 - game.get_tile(LOST).Width()/2,
        WINDOW_HEIGHT/2 - game.get_tile(LOST).Height()/2,game.get_tile(WIN),true);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS; //очищает указанные буферы
      glDrawPixels (WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, screenBuffer.Data()); GL_CHECK_ERRORS;
      for (int i=0;i<256;i++)
        glfwSwapBuffers(window);
      break;
    }  
    /*if (i%10 == 0)
      sleep(1);
    else 
      i++;*/
	}

	glfwTerminate();
	return 0;
}

void Initialize(Game & game)
{
  game.add_map("../resources/labirint/map.txt");

  game.add_room("../resources/labirint/AS_D.txt",D);
  game.add_room("../resources/labirint/DS_B.txt",B);
  game.add_room("../resources/labirint/DW_O.txt",O); 
  game.add_room("../resources/labirint/S_C.txt",C);
  game.add_room("../resources/labirint/WD_E.txt",E); 
  game.add_room("../resources/labirint/WS_F.txt",F); 
  game.add_room("../resources/labirint/ASD_A.txt",A); 
  game.add_room("../resources/labirint/AW_T.txt",T); 
  game.add_room("../resources/labirint/D_V.txt",V); 
  game.add_room("../resources/labirint/WASD_K.txt",K); 
  game.add_room("../resources/labirint/Win.txt",END); 
  game.add_room("../resources/labirint/W.txt",W); 

  game.add_tile(Image("../resources/rooms/floor.png"),FLOOR);
  game.add_tile(Image("../resources/rooms/empty.png"),EMPTY);
  game.add_tile(Image("../resources/rooms/wall.png"),WALL);
  game.add_tile(Image("../resources/rooms/closed_doo.png"),DOOR_CLOSED);
  game.add_tile(Image("../resources/rooms/door_opened.png"),DOOR_OPENED);
  game.add_tile(Image("../resources/rooms/door_opening.png"),DOOR_OPENING);
  game.add_tile(Image("../resources/rooms/key.png"),KEY);

  game.add_tile(Image("../resources/rooms/top.png"),FIRE_TOP);
  game.add_tile(Image("../resources/rooms/right.png"),FIRE_RIGHT);
  game.add_tile(Image("../resources/rooms/left.png"),FIRE_LEFT);
  game.add_tile(Image("../resources/rooms/down.png"),FIRE_DOWN);

  game.add_tile(Image("../resources/bat.png"),ENEMY);
  game.add_tile(Image("../resources/bat_d.png"),ENEMY_DYING);
  game.add_tile(Image("../resources/ghost.png"),BOSS);
  game.add_tile(Image("../resources/ghost_stunned.png"),BOSS_STUNNED);

  game.add_tile(Image("../resources/rooms/finalDoor.png"),FINAL_DOOR);
  game.add_tile(Image("../resources/symbols/letterKeys.png"),letterK);
  game.add_tile(Image("../resources/symbols/letter0.png"),letter0);
  game.add_tile(Image("../resources/symbols/letter1.png"),letter1);
  game.add_tile(Image("../resources/symbols/letter2.png"),letter2);
  game.add_tile(Image("../resources/symbols/letter3.png"),letter3);
  game.add_tile(Image("../resources/symbols/letter4.png"),letter4);
  game.add_tile(Image("../resources/symbols/youwin.png"),WIN);
  game.add_tile(Image("../resources/symbols/youlost.png"),LOST);
}
