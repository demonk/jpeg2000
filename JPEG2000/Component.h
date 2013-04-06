#ifndef COMPONENT
#define COMPONENT
#include "TileCodeParam.h"

class Component
{

public:
	int XRsiz;
	int YRsiz;

	//data 大小 
	int width;
	int height;
	//相对于整个图像的偏移
	int x0,y0;

	int precision;

	int bpp;/*图像深度*/

	int sgnd;

};

#endif