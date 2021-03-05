#ifndef MAIN_GAME_H
#define MAIN_GAME_H

#include "Image.h"
#include "Player.h"
#include <cstring>
#include <vector>
#include <list>
#include <GLFW/glfw3.h>
#include <iostream>

constexpr int FIELD_WIDTH = 32, FIELD_HEIGHT = 25;

struct Player;
struct Game;
enum class MovementDir;


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
};

struct Door
{
  int x;
  int y;
  bool isOpened;
  bool isFinal;
  int roomNumber;
  hero_move move;
};

enum Tiles_type
{
    WALL,
    FLOOR,
    KEY,
    DOOR_CLOSED,
    DOOR_OPENED,
    ENEMY,
    letterK,
    letter0,
    letter1,
    letter2,
    letter3,
    letter4,
    WIN,
    LOST,
    FINAL_DOOR,
    NULL_TERM
}; 

struct Room
{
  Room(){};

  Room(Rooms new_type, Game & game);

  Image & getImage() { return image; }

  std::vector<Wall>   & getWalls()    { return walls; }
  std::list<Key>    & getKeys()     { return keys; }
  std::vector<Enemy>  & getEnemies()  { return enemies; }
  std::vector<Door>   & getDoors()    { return allDoors; }

  bool checkWalls(int &x, int &y, MovementDir lastDir);

  void checkKeys(Player & player);
  void checkDoors(Player & player, Game & game, Image & screen,GLFWwindow*  window);
  void checkEnemies(Player & player);

  ~Room(){};

private:
  Rooms type = NULL_ROOM;
  Image image;
  std::vector<Wall> walls;
  std::list<Key> keys;
  std::vector<Enemy> enemies;
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

#endif