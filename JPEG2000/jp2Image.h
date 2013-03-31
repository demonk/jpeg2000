#ifndef J2K_IMAGE
#define J2K_IMAGE
#include "Component.h"


	class jp2Image
	{
	public:
		int XOsiz;
		int YOsiz;
		int Xsiz;
		int Ysiz;
		int numComponents;
		int colorSpace;

		Component *comps;
	};
#endif