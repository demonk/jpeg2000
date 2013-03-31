#ifndef J2K_Coder
#define J2K_Coder
#include "j2kCustom.h"
#include "jp2Image.h"
#include "CodeParam.h"
#include <iostream>
#include "IOStream.h"
#include "jpegMath.h"
#include "j2kTileCoder.h"
class j2kCoder
{
private:
		jp2Image *image;
		CodeParam *cp;

		j2kTileCoder *tileCoder;

		int state;

		static int currectPos;
		static int currectTileNo;
		static int SOD_Start;
		static int SOT_Start;

public:
	int j2kEncode(jp2Image *img,CodeParam *cp,char *output,int len);

private:
	void writeSOC();
	void writeSIZ();
	void writeSOT();
	void writePOC();
	void writeSOD();
	void writeEOC();
	void writeCOD();
	void writeQCD();
	void writeComment();
	void writeROI(int compno,int tileno);
	void writeQCC(int compno);
	void writeCOC(int compno);
	void writeCOX(int compno);
	void writeQCX(int compno);
};
#endif