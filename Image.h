#ifndef MAIN_IMAGE_H
#define MAIN_IMAGE_H

#include <string>
#include <GLFW/glfw3.h>

constexpr int tileSize = 32;

struct Pixel
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};


constexpr Pixel backgroundColor{0, 0, 0, 0};

struct Image
{
  explicit Image(const std::string &a_path);
  Image(int a_width, int a_height, int a_channels);
  Image(const Image & obj);
  const Image& operator=(const Image & obj);

  int Save(const std::string &a_path);

  int & Width()    { return width; }
  int & Height()   { return height; }
  int Channels() const { return channels; }
  size_t Size()  const { return size; }
  Pixel* Data()        { return  data; }
  void PutImage(int x,int y,Image & picture,bool clear=false);
  void PutImageDirect(int x,int y,Image & picture,bool clear=false);

  Pixel GetPixel(int x, int y) { return data[width * y + x];}
  void  PutPixel(int x, int y, const Pixel &pix) { data[width* y + x] = pix; }

  ~Image();

  Image(){};

private:
  int width = -1;
  int height = -1;
  int channels = 3;
  size_t size = 0;
  Pixel *data = nullptr;
  bool self_allocated = false;
};

Pixel blend(Pixel oldPixel, Pixel newPixel);

#endif //MAIN_IMAGE_H
