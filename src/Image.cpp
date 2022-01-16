#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>


Image::Image(const std::string &a_path)
{
  if((data = (Pixel*)stbi_load(a_path.c_str(), &width, &height, &channels, 4)) != nullptr)
  {
    size = width * height * channels;
  }
}

Image::Image(int a_width, int a_height, int a_channels)
{
  data = new Pixel[a_width * a_height ]{};

  if(data != nullptr)
  {
    width = a_width;
    height = a_height;
    size = a_width * a_height * a_channels;
    channels = a_channels;
    self_allocated = true;
  }
}

Image::Image(const Image & obj)
{
  data = new Pixel[obj.width*obj.height];
  width = obj.width;
  height = obj.height;
  channels = obj.channels;
  size = obj.size;
  self_allocated = true;
  memcpy(data,obj.data,width*height*sizeof(Pixel));
}

const Image& Image::operator=(const Image & obj)
{
  if (data!=nullptr)
    delete [] data;
  data = new Pixel[obj.width*obj.height];
  width = obj.width;
  height = obj.height;
  channels = obj.channels;
  size = obj.size;
  self_allocated = true;
  memcpy(data,obj.data,width*height*sizeof(Pixel));
  return *this;
}

void Image::PutImage(int x,int y,Image & picture, bool clear) //x,y - координаты левого нижнего угла
{
  int pic_width = picture.Width();
  int pic_height = picture.Height();

  if (y + pic_height > height)
    pic_height = height - y;
  if (width - x < pic_width)
    pic_width = width - x;

  if (!clear)
    for (int i=0;i<pic_height;i++)
      memcpy(data + (y+i)*width + x, picture.Data()+(pic_height-i-1)*pic_width,pic_width*sizeof(Pixel));
  else
  {
    for (int i=0;i<pic_height;i++)
      for (int j=0;j<pic_width;j++)
        PutPixel(x+j,y+i,darker(GetPixel(x+j,y+i),picture.GetPixel(j,pic_height-i-1)));
  }
}

void Image::PutImageDirect(int x,int y,Image & picture, bool clear) //x,y - координаты левого нижнего угла
{
  int pic_width = picture.Width();
  int pic_height = picture.Height();

  if (!clear)
    for (int i=0;i<pic_height;i++)
      memcpy(data + (y+i)*width + x, picture.Data()+i*pic_width,pic_width*sizeof(Pixel));
  else
  {
    for (int i=0;i<pic_height;i++)
      for (int j=0;j<pic_width;j++)
        PutPixel(x+j,y+i,darker(GetPixel(x+j,y+i),picture.GetPixel(j,i)));
  }
}


int Image::Save(const std::string &a_path)
{
  auto extPos = a_path.find_last_of('.');
  if(a_path.substr(extPos, std::string::npos) == ".png" || a_path.substr(extPos, std::string::npos) == ".PNG")
  {
    stbi_write_png(a_path.c_str(), width, height, channels, data, width * channels);
  }
  else if(a_path.substr(extPos, std::string::npos) == ".jpg" || a_path.substr(extPos, std::string::npos) == ".JPG" ||
          a_path.substr(extPos, std::string::npos) == ".jpeg" || a_path.substr(extPos, std::string::npos) == ".JPEG")
  {
    stbi_write_jpg(a_path.c_str(), width, height, channels, data, 100);
  }
  else
  {
    std::cerr << "Unknown file extension: " << a_path.substr(extPos, std::string::npos) << "in file name" << a_path << "\n";
    return 1;
  }

  return 0;
}

Image::~Image()
{
  if(self_allocated)
    delete [] data;
  else
  {
    stbi_image_free(data);
  }
}

Pixel darker(Pixel oldpix, Pixel newpix)
{
  newpix.r = newpix.a/255.0*(newpix.r-oldpix.r)+oldpix.r;
  newpix.g = newpix.a/255.0*(newpix.g-oldpix.g)+oldpix.g;
  newpix.b = newpix.a/255.0*(newpix.b-oldpix.b)+oldpix.b;
  newpix.a = 255;
  return newpix;
}