#include "Player.h"
#include <cmath>
#include <GLFW/glfw3.h>

constexpr float DELTATIME = 0.05, BOSS_TIME_STUNNED = 2.0;
constexpr int ENEMY_SPEED = 2; 
constexpr float BOSS_SPEED = 1.3;
constexpr int ENEMY_DISTANCE = 24, KEY_DISTANCE = 32;
constexpr int FIRE_RANGE = 24, EMPTY_RANGE = 16, BOSS_RANGE = 16;


bool Player::Moved() const
{
  if(coords.x == old_coords.x && coords.y == old_coords.y)
    return false;
  else
    return true;
}

void Player::ProcessInput(MovementDir dir)
{
  int move_dist = move_speed * 1;
  doubleLastDir = lastDir;
  lastDir = dir;
  switch(dir)
  {
    case MovementDir::UP:
      old_coords.y = coords.y;
      coords.y += move_dist;
      break;
    case MovementDir::DOWN:
      old_coords.y = coords.y;
      coords.y -= move_dist;
      break;
    case MovementDir::LEFT:
      old_coords.x = coords.x;
      coords.x -= move_dist;
      break;
    case MovementDir::RIGHT:
      old_coords.x = coords.x;
      coords.x += move_dist;
      break;
    default:
      break;
  }
}

void Room::checkDoorOpening(Game & game)
{
  std::vector<Door>::iterator iterDoors = getDoors().begin();
  while (iterDoors!=getDoors().end())
  {
    if (iterDoors->isOpening && iterDoors->move.x == iterDoors->move.size-1)
    {
      iterDoors->isOpening = false;
      iterDoors->move.direct = &game.get_tile(DOOR_OPENED);
      iterDoors->move.x = 0;
      iterDoors->move.size = 6;
      iterDoors->move.last_time= glfwGetTime();
    }
    iterDoors++;
  }
}

void Room::checkFire(Player & player, Game & game)
{
  bool end = false;
  if (player.getLastAction()==MovementDir::Q)
  {
    switch (player.getLastDir())
    {
      case MovementDir::UP :
        fireballs.push_back(Fire{player.getX(), player.getY(), MovementDir::UP, {&game.get_tile(FIRE_TOP), 
          0, 6, (GLfloat)glfwGetTime()}});
        break;
      case MovementDir::DOWN :
        fireballs.push_back(Fire{player.getX(), player.getY(), MovementDir::DOWN, {&game.get_tile(FIRE_DOWN), 
          0, 6, (GLfloat)glfwGetTime()}});
        break;
      case MovementDir::LEFT :
        fireballs.push_back(Fire{player.getX(), player.getY(), MovementDir::LEFT, {&game.get_tile(FIRE_LEFT), 
          0, 6, (GLfloat)glfwGetTime()}});
        break;
      case MovementDir::RIGHT :
        fireballs.push_back(Fire{player.getX(), player.getY(), MovementDir::RIGHT, {&game.get_tile(FIRE_RIGHT), 
          0, 6, (GLfloat)glfwGetTime()}});
        break;
    }
    player.getLastAction() = MovementDir::BLANK;
  }
  std::list<Fire>::iterator iterFire = fireballs.begin();
  std::list<Enemy>::iterator iterEnemies;
  while (iterFire != fireballs.end())
  {
    switch(iterFire->dir)
    {
      case MovementDir::UP :
        iterFire->y += ENEMY_SPEED;
        break;
      case MovementDir::DOWN :
        iterFire->y -= ENEMY_SPEED;
        break;
      case MovementDir::LEFT :
        iterFire->x -= ENEMY_SPEED;
        break;
      case MovementDir::RIGHT :
        iterFire->x += ENEMY_SPEED;
        break;
    }

    end = checkWalls(iterFire->x, iterFire->y, iterFire->dir);

    iterEnemies = enemies.begin();
    while (iterEnemies!=enemies.end())
    {
      if (iterEnemies->x - iterFire->x < FIRE_RANGE && iterEnemies->x - iterFire->x > -FIRE_RANGE
        && iterEnemies->y - iterFire->y < FIRE_RANGE && iterEnemies->y - iterFire->y > -FIRE_RANGE)
      {
        iterEnemies->isDying = true;
        iterEnemies->move.direct = &game.get_tile(ENEMY_DYING);
        iterEnemies->move.x = 0;
        iterEnemies->move.size = 6;
        iterEnemies->move.last_time = (GLfloat)glfwGetTime();

        end = true;
        break;
      }
      iterEnemies++;
    }

    if (isFinal)
    {
      if (boss.x - iterFire->x < FIRE_RANGE && boss.x - iterFire->x > -FIRE_RANGE
        && boss.y - iterFire->y < FIRE_RANGE && boss.y - iterFire->y > -FIRE_RANGE) 
        {
          boss.isStunned = true;
          boss.timeStunned = (GLfloat)glfwGetTime();
          boss.move.direct = &game.get_tile(BOSS_STUNNED);
          boss.move.x = 0;
          boss.move.size = 2;
          boss.move.last_time = (GLfloat)glfwGetTime();
        }
    }

    if (end)
      iterFire = fireballs.erase(iterFire);
    else 
      iterFire++;
    
  }

}

void Room::checkEmpties(Player & player)
{
  std::vector<Empty>::iterator iterEmpties = empties.begin();
  while (iterEmpties!=empties.end())
  {
    if (player.getX() - iterEmpties->x < EMPTY_RANGE && player.getX() - iterEmpties->x > -EMPTY_RANGE
      && player.getY() - iterEmpties->y < EMPTY_RANGE && player.getY() - iterEmpties->y > -EMPTY_RANGE)
    {
      player.getStatus() = true;
      break;
    }
    iterEmpties++;
  }
}

void Room::checkBoss(Player & player, Game & game)
{
  double x, y, x2, y2, z1, z2, K, buf;
  if (boss.isStunned)
  {
    if (glfwGetTime() - boss.timeStunned >= BOSS_TIME_STUNNED)
    {
      boss.isStunned = false;
      boss.move.direct = &game.get_tile(BOSS);
      boss.move.x = 0;
      boss.move.size = 10;
      boss.move.last_time = (GLfloat)glfwGetTime();
    }
    else 
      return;
  }


  if (player.getX() == boss.x)
  {
    if (player.getY() > boss.y)
      boss.y+=BOSS_SPEED;
    else
      boss.y-=BOSS_SPEED;
  }
  else 
  {
    K = (boss.y - player.getY())/(boss.x - player.getX());
    buf = std::sqrt((K*K*boss.x+boss.x)*(K*K*boss.x+boss.x) -(1+K*K)*(boss.x*boss.x*K*K - BOSS_SPEED*BOSS_SPEED + boss.x*boss.x));
    z1 = (boss.x*K*K + boss.x + buf)/(1+K*K);
    z2 = (boss.x*K*K + boss.x - buf)/(1+K*K);
    y = boss.x;
    if (boss.x > player.getX())
    {
      if (y >= z1)
        x = z1;
      else 
        x = z2;
    }
    else {
      if (y < z1)
        x = z1;
      else 
        x = z2;
    }
    y = (boss.y - player.getY())*1.0/(boss.x - player.getX()) * x + 
      (boss.y - (boss.y - player.getY())*1.0/(boss.x - player.getX())*boss.x);
    

    boss.x = modf(x,&K)>0.5 ? K+1 : K;
    boss.y = modf(y,&K)>0.5 ? K+1 : K;

  }
  
  if (player.getX() - boss.x < BOSS_RANGE && player.getX() - boss.x > -BOSS_RANGE && 
    player.getY() - boss.y < BOSS_RANGE && player.getY() - boss.y > -BOSS_RANGE)
    player.getStatus() = true;
}
void Player::Draw(Image & screen, Game & game, GLFWwindow*  window) 
{
  int width = 24;
  int height = 32;

  Pixel old[4];
  Pixel new_pix;

  game.getCurRoom().checkWalls(coords.x, coords.y, lastDir);
  game.getCurRoom().checkEmpties(*this);
  game.getCurRoom().checkKeys(*this);
  game.getCurRoom().checkEnemies(*this);

  if (game.getCurRoom().final())
    game.getCurRoom().checkBoss(*this, game);

  game.getCurRoom().checkDoorOpening(game);
  game.getCurRoom().checkFire(*this, game);
  game.getCurRoom().checkDoors(*this, game, screen,window);

  screen.PutImageDirect(0,0,game.getCurRoom().getImage());
  Image heroCur(24,32,4);

  switch (lastDir)
  {
    case MovementDir::UP :
      getHeroMove(hero_top,heroCur);
      break;
    case MovementDir::DOWN :
      getHeroMove(hero_down,heroCur);
      break;
    case MovementDir::RIGHT :
      getHeroMove(hero_right,heroCur);
      break;
    case MovementDir::LEFT :
      getHeroMove(hero_left,heroCur);
      break;
    default :
      getHeroMove(hero_stay,heroCur);
      break;
  }
  
  for(int y = coords.y; y < coords.y + height; y++)
  {
    for(int x = coords.x; x < coords.x + width; x++)
    {
      new_pix = heroCur.GetPixel(x - coords.x, coords.y + height - y - 1);
      screen.PutPixel(x+4, y, darker(screen.GetPixel(x+4,y),new_pix)); //прорисовка персонажа
    }
  }

  drawObjects<std::list<Key>>(game.getCurRoom().getKeys().begin(),
    game.getCurRoom().getKeys().end(), screen, 31, 31, 0.5);

  drawObjects<std::vector<Door>>(game.getCurRoom().getDoors().begin(),
    game.getCurRoom().getDoors().end(), screen, 32, 32);

  drawObjects<std::list<Enemy>>(game.getCurRoom().getEnemies().begin(),
    game.getCurRoom().getEnemies().end(), screen, 32, 32, 2.0);

  drawObjects<std::list<Fire>>(game.getCurRoom().getFire().begin(),
    game.getCurRoom().getFire().end(), screen, 32, 32, 2.0);

  //прорисовка босса
  if (game.getCurRoom().final())
  {
    Image boss(32,32,4);
    getHeroMove(game.getCurRoom().getBoss().move, boss);
    screen.PutImage(game.getCurRoom().getBoss().x, game.getCurRoom().getBoss().y, boss, true);
  }
  

  //рисуем поле
  
  screen.PutImage(FIELD_WIDTH*32+32,FIELD_HEIGHT*32-64,game.get_tile(letterK));
  switch(keys)
  {
    case 0:
      screen.PutImage(FIELD_WIDTH*32+32+200,FIELD_HEIGHT*32-64,game.get_tile(letter0));
      break;
    case 1:
      screen.PutImage(FIELD_WIDTH*32+32+200,FIELD_HEIGHT*32-64,game.get_tile(letter1));
      break;
    case 2:
      screen.PutImage(FIELD_WIDTH*32+32+200,FIELD_HEIGHT*32-64,game.get_tile(letter2));
      break;
    case 3:
      screen.PutImage(FIELD_WIDTH*32+32+200,FIELD_HEIGHT*32-64,game.get_tile(letter3));
      break;
    case 4:
      screen.PutImage(FIELD_WIDTH*32+32+200,FIELD_HEIGHT*32-64,game.get_tile(letter4));
      break;
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
  map = new char[512];
  FILE * labirint = fopen(txt_path,"r");
  int size = fread(map,sizeof(char),50,labirint);
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
    if (x_center - iterWall->x < KEY_DISTANCE && x_center - iterWall->x > -KEY_DISTANCE 
    && y_center - iterWall->y < KEY_DISTANCE && y_center - iterWall->y > -KEY_DISTANCE)
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

void Player::checkCoord()
{
  if (lastDir == MovementDir::BLANK && doubleLastDir!=MovementDir::BLANK)
  {
    switch(doubleLastDir)
    {
      case MovementDir::UP:
        coords.y = (coords.y % 32 - 32/2 > 0) ? (int)(coords.y / 32)*32 + 32 : coords.y ;
        break;
      case MovementDir::DOWN:
        coords.y = (int)(coords.y/32) * 32 ;
        break;
      case MovementDir::RIGHT:
        coords.x = (coords.x % 32 - 32/2 > 0) ? (int)(coords.x / 32)*32 + 32 : coords.x ;
        break;
      case MovementDir::LEFT:
        coords.x = (int)(coords.x / 32)*32;
        break;
    }
  }
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
      iterDoor->move.size = 5;
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
  player.getLastAction()=MovementDir::BLANK;

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

    drawObjects<std::list<Enemy>>(getCurRoom().getEnemies().begin(),
    getCurRoom().getEnemies().end(), screen, 32, 32, 4.0);

    getHeroMove(player.get_hero_stay(),heroCur);
    for(int y = player.getY(); y < player.getY() + player.get_hero().Height(); y++)
    {
      for(int x = player.getX(); x < player.getX() + player.get_hero().Width(); x++)
      {
        buf = heroCur.GetPixel(x - player.getX(), player.getY() + player.get_hero().Height() - y - 1);
        screen.PutPixel(x+4, y, darker(screen.GetPixel(x+4,y),buf)); //прорисовка персонажа
      }
    }
    fadeScreen(screen, window, 100 - i);
    screen = temp;
  }
}

void Room::checkDoors(Player & player, Game & game, Image & screen, GLFWwindow*  window)
{
  if (player.getLastAction()!=MovementDir::E)
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
        iterDoors->isOpening = true;
        iterDoors->isOpened = true;
        iterDoors->move.direct = &game.get_tile(DOOR_OPENING);
        iterDoors->move.x = 0;
        iterDoors->move.size = 14;
        iterDoors->move.last_time= glfwGetTime();
        player.getKey()--;
      }
      else if (iterDoors->isOpened && !iterDoors->isOpening)
      {
        game.toNewRoom(iterDoors->roomNumber, player, screen, window);
      }
      break;
    }
    iterDoors++;
  }
  player.getLastAction() = MovementDir::BLANK;
}

void Room::checkEnemies(Player & player)
{
  int x_center = player.getX()+16;
  int y_center = player.getY()+16;
  int enX_center, enY_center;
  int speed;
  std::list<Enemy>::iterator iterEnemies = enemies.begin();

  while(iterEnemies!=enemies.end())
  {
    speed = iterEnemies->speed;

    if (iterEnemies->isDying)
    {
      if (iterEnemies->move.x == iterEnemies->move.size - 1)
        iterEnemies = enemies.erase(iterEnemies);
      else 
        iterEnemies++;
      continue;
    }
      
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

    if (x_center - enX_center < ENEMY_DISTANCE && x_center - enX_center > -ENEMY_DISTANCE 
    && y_center - enY_center < ENEMY_DISTANCE && y_center - enY_center > -ENEMY_DISTANCE)
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

//создает новую комнату в игре

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
          allDoors.push_back(Door{j*32,i*32,false, false, 0, {&game.get_tile(DOOR_CLOSED), 0, 8, (GLfloat)glfwGetTime()}});
          walls.push_back(Wall{j*32,i*32});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'E':
          my_tile = ENEMY;
          enemies.push_back(Enemy{j*32,i*32,false, true, {&game.get_tile(ENEMY), 0, 7, (GLfloat)glfwGetTime()}, ENEMY_SPEED});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'Y':
          my_tile = ENEMY;
          enemies.push_back(Enemy{j*32,i*32,true, true,{&game.get_tile(ENEMY), 0, 7, (GLfloat)glfwGetTime()}, ENEMY_SPEED});
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case 'B':
          my_tile = BOSS;
          isFinal = true;
          boss = Boss{j*32,i*32,{&game.get_tile(BOSS), 0, 10, (GLfloat)glfwGetTime()}, false, (int)BOSS_SPEED, 0};
          image.PutImage(j*32,i*32,game.get_tile(FLOOR));
          break;
        case '_':
          my_tile = EMPTY;
          empties.push_back(Empty{j*32,i*32});
          image.PutImage(j*32,i*32,game.get_tile(EMPTY));
          break;
        case '@':
          player_x = j*32;
          player_y = i*32;
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
  int i = 0;
  int height = 0, width = 0;
  while (map[i]!=' ')
  {
    height = height*10 + map[i]-'0';
    i++;
  }
  i++;
  while (map[i]!='\n')
  {
    width = width*10 + map[i]-'0';
    i++;
  }
  i++;
  cur_room = 0;
  while (map[i]!='\n')
  {
    cur_room = cur_room*10 + map[i]-'0';
    i++;
  }
  if (cur_room >= width*height)
  {
    std::cout<<"Wrong number of starting room\n";
    exit(-1);
  }
  i++;
  map = map+i;
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

