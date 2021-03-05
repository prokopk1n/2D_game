#ifndef MAIN_PLAYER_H
#define MAIN_PLAYER_H

#include "Image.h"
#include <cstring>
#include <vector>
#include <list>
#include <GLFW/glfw3.h>
#include <iostream>

struct hero_move
{
  Image * direct;
  int x = 0;
  int size = 6;
  GLfloat last_time;
};

enum class MovementDir
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  E,
  Q,
  BLANK
};

constexpr int FIELD_WIDTH = 32, FIELD_HEIGHT = 25;

struct Player;
struct Game;

enum Rooms
{
  A,
  B,
  C,
  D,
  E,
  K,
  T,
  F,
  O,
  W,
  V,
  END,
  NULL_ROOM
};

enum Side
{
  West,
  South,
  North,
  East
};

struct Fire
{
  int x;
  int y;
  MovementDir dir;
  hero_move move;
};

struct Wall
{
  int x;
  int y;
};

struct Key
{
  int x;
  int y;
  hero_move move;
};

struct Enemy
{
  int x;
  int y;
  bool isUp;
  bool isIncrease;
  hero_move move;
  int speed;
  bool isDying = false;
};

struct Door
{
  int x;
  int y;
  bool isOpened;
  bool isFinal;
  int roomNumber;
  hero_move move;
  bool isOpening;
};

struct Empty
{
  int x;
  int y;
};

enum Tiles_type
{
    WALL,
    FLOOR,
    KEY,
    DOOR_CLOSED,
    DOOR_OPENED,
    DOOR_OPENING,
    ENEMY,
    ENEMY_DYING,
    FIRE_TOP,
    FIRE_DOWN,
    FIRE_LEFT,
    FIRE_RIGHT,
    BOSS,
    BOSS_STUNNED,
    letterK,
    letter0,
    letter1,
    letter2,
    letter3,
    letter4,
    EMPTY,
    WIN,
    LOST,
    FINAL_DOOR,
    NULL_TERM
}; 

struct Boss
{
  int x;
  int y;
  hero_move move;
  bool isStunned;
  int speed;
  GLfloat timeStunned;
};

struct Room
{
  Room(){};

  Room(Rooms new_type, Game & game);

  int & getPlayerX() { return player_x; }
  int & getPlayerY() { return player_y; }

  Image & getImage() { return image; }

  std::vector<Wall> & getWalls()   { return walls; }
  std::list<Key>    & getKeys()    { return keys; }
  std::list<Enemy>  & getEnemies() { return enemies; }
  std::vector<Door> & getDoors()   { return allDoors; }
  std::list<Fire>   & getFire()    { return fireballs; }
  Boss              & getBoss()    { return boss; }

  bool final() { return isFinal; }

  bool checkWalls(int &x, int &y, MovementDir lastDir);
  void checkKeys(Player & player);
  void checkDoors(Player & player, Game & game, Image & screen,GLFWwindow*  window);
  void checkEnemies(Player & player);
  void checkDoorOpening(Game & game);
  void checkFire(Player & player, Game & game);
  void checkEmpties(Player & player);
  void checkBoss(Player & player, Game & game);

  ~Room(){};

private:
  int player_x, player_y;
  Rooms type = NULL_ROOM;
  Image image;
  bool isFinal = false;
  Boss boss;
  std::vector<Wall> walls;
  std::vector<Empty> empties;
  std::list<Key> keys;
  std::list<Fire> fireballs;
  std::list<Enemy> enemies;
  std::vector<Door> allDoors;
  
};

struct Game
{   
    void add_tile(const Image & tile,Tiles_type num);
    Image& get_tile(Tiles_type num){ return tiles[num]; }

    void add_map(const char * txt_path);
    char * get_map() { return map; }

    void add_room(const char * txt_path, Rooms num);
    char * get_room(Rooms num) { return room[num]; }

    std::vector<Room> & getRoomVector() { return rooms; }
    Room & getCurRoom() { return rooms[cur_room]; }
    int & CurRoomNumber() { return cur_room; }

    void addLoading(const Image & tile);

    void createRoomVector();
    
    void toNewRoom(int number, Player & player, Image & screen, GLFWwindow*  window);

private:
    char * map = nullptr;
    char ** room = nullptr;
    Image * tiles = nullptr;
    Image loading;
    std::vector<Room> rooms;
    int cur_room = 0;
};

template < typename T >
void drawObjects(typename T::iterator begin, typename T::iterator end, 
  Image & screen, int width, int height, double coeff = 1.0);


int getNeigh(int x, int y, int width, int height, Side side);

void getHeroMove(hero_move & move, Image & image, double coeff = 1);

void fadeScreen(Image & screen, GLFWwindow*  window, int x);

struct Point
{
  int x;
  int y;
};

struct Player
{
  explicit Player() : keys(0), isLost(false), isWin(false)  {};

  bool Moved() const;
  void ProcessInput(MovementDir dir);
  void Draw(Image & screen, Game & game, GLFWwindow*  window);
  Image& get_hero() { return hero; }

  bool & getStatus() { return isLost; }
  bool & getWinStatus() { return isWin; }

  int & getKey() { return keys; }

  hero_move & get_hero_stay() { return hero_stay; }
  hero_move & get_hero_left() { return hero_left; }
  hero_move & get_hero_right() { return hero_right; }
  hero_move & get_hero_top() { return hero_top; }
  hero_move & get_hero_down() { return hero_down; }

  MovementDir & getLastDir() { return lastDir; }  
  MovementDir & getDLastDir() { return doubleLastDir; }
  MovementDir & getLastAction() { return lastAction; }

  void checkCoord();

  int & getX() { return coords.x; }
  int & getY() { return coords.y; }

private:
  Point coords {.x = 10, .y = 10};
  Point old_coords {.x = 10, .y = 10};
  Pixel color {.r = 255, .g = 255, .b = 0, .a = 255};
  int move_speed = 2;
  Image hero;
  hero_move hero_left, hero_right,
     hero_top, hero_down, hero_stay;
  int keys = 0;
  bool isLost = false;
  bool isWin = false;
  MovementDir lastDir, doubleLastDir;
  MovementDir lastAction;
};

#endif //MAIN_PLAYER_H
