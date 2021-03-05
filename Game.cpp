#include "Game.h"
#include <GLFW/glfw3.h>

constexpr float DELTATIME = 0.05;
constexpr int ENEMY_SPEED = 2;

Room::Room(Rooms new_type, Game & game):image(FIELD_WIDTH*32,FIELD_HEIGHT*32, 4)
{
  char * new_map = game.get_room(new_type);
  type = new_type;
  char tile_type;
  Tiles_type my_tile;
  int roomNumber;
  char * new_room = new_map;
  for (int i=0;i<FIELD_HEIGHT;i++)
    for (int j=0;j<FIELD_WIDTH;j++)
    {
      tile_type = new_room[(FIELD_HEIGHT-i-1)*FIELD_WIDTH+j];
      switch(tile_type)
      {
        case '#':
          my_tile = WALL;
          walls.push_back(Wall{j*32,i*32});
          image.PutImage(j*32,i*32,game.get_tile(my_tile));
          break;
        case ' ':
          my_tile = FLOOR;
          image.PutImage(j*32,i*32,game.get_tile(my_tile));
          break;
        case 'K':
          my_tile = KEY;
          keys.push_back(Key{j*32,i*32, {&game.get_tile(KEY), 0, 6, (GLfloat)glfwGetTime()}});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'x':
          my_tile = DOOR_OPENED;
          allDoors.push_back(Door{j*32,i*32,false, false, 0, {&game.get_tile(DOOR_CLOSED), 0, 10, (GLfloat)glfwGetTime()}});
          walls.push_back(Wall{j*32,i*32});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'E':
          my_tile = ENEMY;
          enemies.push_back(Enemy{j*32,i*32,false, true, {&game.get_tile(ENEMY), 0, 4, (GLfloat)glfwGetTime()}, ENEMY_SPEED});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'Y':
          my_tile = ENEMY;
          enemies.push_back(Enemy{j*32,i*32,true, true,{&game.get_tile(ENEMY), 0, 4, (GLfloat)glfwGetTime()}, ENEMY_SPEED});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'X':
          my_tile = FINAL_DOOR;
          allDoors.push_back(Door{j*32,i*32,false, true, 0, {&game.get_tile(FINAL_DOOR), 0, 1, (GLfloat)glfwGetTime()}});
          walls.push_back(Wall{j*32,i*32});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          //image.PutImage(j*32,i*32,game.get_tile(FINAL_DOOR), true);
          break;
      }
    }
};

void Game::createRoomVector()
{
  char * map = get_map();
  int height = map[0] - '0';
  int width  = map[2] - '0';
  cur_room   = map[4] - '0';

  map = map+6;
  Rooms type;
  std::vector<Door>::iterator iterDoor;
  for (int i=0;i<height;i++)
  {
    for (int j=0;j<width;j++)
    {
      switch (*map)
      {
        case 'A':
          type = A;
          break;
        case 'B':
          type = B;
          break;
        case 'C':
          type = C;
          break;
        case 'D':
          type = D;
          break;
        case 'E':
          type = E;
          break;
        case 'K':
          type = K;
          break;
        case 'T':
          type = T;
          break;
        case 'F':
          type = F;
          break;
        case '*':
          type = END;
          break;
        case 'O':
          type = O;
          break;
        case 'W':
          type = W;
          break;
        case 'V':
          type = V;
          break;
      }
      rooms.push_back(Room{type,*this});
      map++;
      std::vector<Door> & doors = rooms[i*width+j].getDoors();
      iterDoor = doors.begin();
      while(iterDoor!=doors.end())
      {
        if (iterDoor->x == 0)
          iterDoor->roomNumber = getNeigh(j,i,width,height,West);
        else if(iterDoor->x == FIELD_WIDTH*32 - 32)
          iterDoor->roomNumber = getNeigh(j,i,width,height,East);
        else if(iterDoor->y == 0)
          iterDoor->roomNumber = getNeigh(j,i,width,height,South);
        else 
          iterDoor->roomNumber = getNeigh(j,i,width,height,North);
        iterDoor++;
      }
    }
    map++;
  }
}

int getNeigh(int x, int y, int width, int height, Side side)
{
  if (x != 0 && y!=0)
  { 
    switch(side)
    {
      case West:
        return y*width + x - 1;
      case East:
        return y*width + x + 1;
      case North:
        return (y-1)*width + x;
      case South:
        return (y+1)*width + x;  
    }
  }

  else if (x == 0)
  {
    switch(side)
    {
      case East:
        return y*width + x + 1;
      case North:
        if (y!=0)
          return (y-1)*width + x;
        break;
      case South:
        if (y!=height-1)
          return (y+1)*width + x;
        break;
    }
  }

  else if (x == width - 1)
  {
    switch(side)
    {
      case West:
        return y*width + x - 1;
      case North:
        if (y!=0)
          return (y-1)*width + x;
        break;
      case South:
        if (y!=height-1)
          return (y+1)*width + x;
        break;
    }
  }

  else if (y == 0)
  {
    switch(side)
    {
      case West:
        return y*width + x - 1;
      case East:
        return y*width + x + 1;
      case South:
        return (y+1)*width + x;
    }
  }

  else if (y==height - 1)
  {
    switch(side)
    {
      case West:
        return y*width + x - 1;
      case East:
        return y*width + x + 1;
      case North:
        return (y-1)*width + x;
    }
  }

  return -1;
}

template < typename T >
void drawObjects(typename T::iterator begin, typename T::iterator end, 
  Image & screen, int width, int height, double coeff)
{
  Image myImage(width, height, 4);
  while(begin!=end)
  {
    getHeroMove(begin->move, myImage, coeff);
    screen.PutImage(begin->x, begin->y, myImage, true);
    begin++;
  }
}

void Game::add_map(const char * txt_path)
{
  map = new char[256];
  FILE * labirint = fopen(txt_path,"r");
  fread(map,sizeof(char),256,labirint);
  fclose(labirint);
  cur_room = map[4] - '0';
}

void Game::add_tile(const Image & tile,Tiles_type num)
{
  if (tiles == nullptr)
    tiles = new Image[NULL_TERM];
  tiles[num] = tile;
}

//добавляет новый тип комнаты
void Game::add_room(const char * txt_path, Rooms num)
{
  if (room == nullptr)
    room = new char*[NULL_ROOM];
  
  room[num] = new char[FIELD_WIDTH*FIELD_HEIGHT];
  char c;
  FILE * map = fopen(txt_path,"r");
  int j = 0;
  for (int i=0;i<FIELD_HEIGHT;i++)
  {
    fread(room[num]+j,sizeof(char),FIELD_WIDTH,map);
    j+=FIELD_WIDTH;
    fseek(map,1,SEEK_CUR);
  }
  fclose(map);
}

bool Room::checkWalls(int & x, int & y, MovementDir lastDir)
{
  Wall bufWall;
  int i = 0;
  std::vector<Wall>::iterator iterWall = getWalls().begin();
  while (iterWall!=getWalls().end())
  {
    bufWall = *iterWall;
    if (bufWall.y >= y + 32)
      break;
    if (!(x >= bufWall.x + 32 || y >= bufWall.y + 32 || x + 32 <= bufWall.x || y + 32 <= bufWall.y)) 
    {
      switch(lastDir)
      {
        case MovementDir::UP:
          y = bufWall.y - 32;
          break;
        case MovementDir::DOWN:
          y = bufWall.y + 32;
          break;
        case MovementDir::LEFT:
          x = bufWall.x + 32;
          break;
        case MovementDir::RIGHT:
          x = bufWall.x - 32;
          break;
      }
      return true;
    }
    iterWall++;
  }
  return false;
}

void Room::checkKeys(Player & player)
{
  Key bufKey;
  int x_center = player.getX()+16;
  int y_center = player.getY()+16;
  std::list<Key>::iterator iterWall = getKeys().begin();
  while (iterWall!=getKeys().end())
  {
    if (x_center - iterWall->x < 24 && x_center - iterWall->x > -24 && y_center - iterWall->y < 24 
        && y_center - iterWall->y > -24)
    {
      player.getKey()++;
      iterWall = getKeys().erase(iterWall);
    }
    else
      iterWall++;
  }
}

void fadeScreen(Image & screen, GLFWwindow*  window, int x)
{
  Pixel buf_temp, buf;
  Image temp(screen.Width(), screen.Height(), screen.Channels());
  for (int j=0;j<screen.Height();j++)
    for (int k=0;k<screen.Width();k++)
    {
      buf = screen.GetPixel(k,j);
      buf_temp.r = buf.r*(1.0 - x / 100.0);
      buf_temp.g = buf.g*(1.0 - x / 100.0);
      buf_temp.b = buf.b*(1.0 - x / 100.0);
      buf_temp.a = buf.a;
      temp.PutPixel(k,j,buf_temp);
    }
  glDrawPixels (temp.Width(), temp.Height(), GL_RGBA, GL_UNSIGNED_BYTE, temp.Data());
  glfwSwapBuffers(window);
}

void Game::toNewRoom(int number, Player & player, Image & screen, GLFWwindow*  window)
{
  for (int i=0;i<=100;i+=4)
    fadeScreen(screen, window, i); //затемняем старый экран

  std::vector<Room> & myRooms = getRoomVector();
  std::vector<Door>::iterator iterDoor = myRooms[number].getDoors().begin();
  while (iterDoor!=myRooms[number].getDoors().end())
  {
    if (iterDoor->roomNumber==cur_room)
    {
      iterDoor->move.direct = &get_tile(DOOR_OPENED);
      iterDoor->move.x = 0;
      iterDoor->move.size = 1;
      iterDoor->move.last_time= glfwGetTime();
      iterDoor->isOpened = true;
      cur_room = number;
      break;
    }
    iterDoor++;
  }

  if (iterDoor->x==0)
  {
    player.getX() = 32; 
    player.getY() = iterDoor->y;
  }
  else if (iterDoor->x==FIELD_WIDTH*32-32)
  {
    player.getX() = iterDoor->x - 32; 
    player.getY() = iterDoor->y;
  }
  else if (iterDoor->y==0)
  {
    player.getX() = iterDoor->x; 
    player.getY() = iterDoor->y+32;
  }
  else
  {
    player.getX() = iterDoor->x; 
    player.getY() = iterDoor->y - 32;
  }
  player.getLastDir()=MovementDir::BLANK;

  screen.PutImageDirect(0,0,rooms[cur_room].getImage());

  Image heroCur(24,32,4);
  Pixel buf, buf_temp;
  Image temp = screen;

  for (int i=0;i<=100;i+=2)
  {
    getCurRoom().checkKeys(player);
    getCurRoom().checkEnemies(player);
    getCurRoom().checkDoors(player, *this, screen, window);

    drawObjects<std::list<Key>>(getCurRoom().getKeys().begin(),
    getCurRoom().getKeys().end(), screen, 31, 31, 0.5);

    drawObjects<std::vector<Door>>(getCurRoom().getDoors().begin(),
    getCurRoom().getDoors().end(), screen, 32, 32);

    drawObjects<std::vector<Enemy>>(getCurRoom().getEnemies().begin(),
    getCurRoom().getEnemies().end(), screen, 32, 32, 4.0);

    getHeroMove(player.get_hero_stay(),heroCur);
    for(int y = player.getY(); y < player.getY() + player.get_hero().Height(); y++)
    {
      for(int x = player.getX(); x < player.getX() + player.get_hero().Width(); x++)
      {
        buf = heroCur.GetPixel(x - player.getX(), player.getY() + player.get_hero().Height() - y - 1);
        screen.PutPixel(x+4, y, blend(screen.GetPixel(x+4,y),buf)); //прорисовка персонажа
      }
    }
    fadeScreen(screen, window, 100 - i);
    screen = temp;
  }
}

void Room::checkDoors(Player & player, Game & game, Image & screen, GLFWwindow*  window)
{
  if (player.getLastDir()!=MovementDir::E)
    return;
  int x_center = player.getX()+16;
  int y_center = player.getY()+16;
  std::vector<Door>::iterator iterDoors = getDoors().begin();
  while (iterDoors!=getDoors().end())
  {
    if (x_center - (iterDoors->x+16) < 48 && x_center - (iterDoors->x+16) > -48 && y_center - (iterDoors->y+16) < 48 
      && y_center - (iterDoors->y+16) > -48)
    {
      if (iterDoors->isFinal && player.getKey()!=0)
      {
        player.getKey()--;
        player.getWinStatus() = true;
      }
      else if (iterDoors->isOpened == false && player.getKey()!=0)
      {
        iterDoors->isOpened = true;
        iterDoors->move.direct = &game.get_tile(DOOR_OPENED);
        iterDoors->move.x = 0;
        iterDoors->move.size = 1;
        iterDoors->move.last_time= glfwGetTime();
        player.getKey()--;
      }
      else if (iterDoors->isOpened)
      {
        game.toNewRoom(iterDoors->roomNumber, player, screen, window);
      }
      break;
    }
    iterDoors++;
  }
  player.getLastDir() = MovementDir::BLANK;
}

void Room::checkEnemies(Player & player)
{
  int x_center = player.getX()+16;
  int y_center = player.getY()+16;
  int enX_center, enY_center;
  int speed;
  std::vector<Enemy>::iterator iterEnemies = enemies.begin();

  while(iterEnemies!=enemies.end())
  {
    speed = iterEnemies->speed;
    if (iterEnemies->isUp)
    {
      if (iterEnemies->isIncrease)
      {
        iterEnemies->y+=speed;
        if (checkWalls(iterEnemies->x, iterEnemies->y, MovementDir::UP))
          iterEnemies->isIncrease = false;
      }
      else 
      {
        iterEnemies->y-=speed;
        if (checkWalls(iterEnemies->x, iterEnemies->y, MovementDir::DOWN))
          iterEnemies->isIncrease = true;
      }
    }
    else 
    {
      if (iterEnemies->isIncrease)
      {
        iterEnemies->x+=speed;
        if (checkWalls(iterEnemies->x, iterEnemies->y, MovementDir::RIGHT))
          iterEnemies->isIncrease = false;
      }
      else 
      {
        iterEnemies->x-=speed;
        if (checkWalls(iterEnemies->x, iterEnemies->y, MovementDir::LEFT))
          iterEnemies->isIncrease = true;
      }
    }

    enX_center = iterEnemies->x+16;
    enY_center = iterEnemies->y+16;

    if (x_center - enX_center < 48 && x_center - enX_center > -48 && y_center - enY_center < 48 
        && y_center - enY_center > -48)
        player.getStatus() = true;
      
    iterEnemies++;
  }
}

void getHeroMove(hero_move & move, Image & image, double coeff)
{
  GLfloat cur_time = glfwGetTime();
  GLfloat delta = cur_time - move.last_time;
  if (delta > DELTATIME/coeff)
  {
    move.last_time = cur_time;
    move.x++;
    if (move.x == move.size)
      move.x = 0;
  }
  
  for (int i=0;i<image.Height();i++)
    for (int j=0;j<image.Width();j++)
      image.PutPixel(j,i,move.direct->GetPixel(j+image.Width()*move.x, i));
  
}