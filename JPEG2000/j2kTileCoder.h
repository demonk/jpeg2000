#ifndef J2K_TILE_CODER
#define J2K_TILE_CODER
#include <stdlib.h>
#include "j2kCustom.h"
#include "CodeParam.h"
#include "TileCodeParam.h"
#include "jp2Image.h"
#include "StepSize.h"
#include "j2kTag.h"
#include "jpegMath.h"

/************************************************************************/
/* 编码块信息定义                                                                      */
/************************************************************************/
class j2kTileCoder_CodeBlock
{
public:
	int x0,y0;
	int x1,y1;
	int numbps;
};


class j2kTileCoder_Precinct
{
public:
	int x0,y0;
	int x1,y1;
	int codeBlockNumInWidth;
	int codeBlockNumInHeight;
	int width,height;
	j2kTileCoder_CodeBlock *codeBlockInfo;
	j2kTagTree *inclusionTree;
	j2kTagTree *imsbTree;
};

/************************************************************************/
/* 子带信息定义                                                                     */
/************************************************************************/
class j2kTileCoder_Band
{
public:
	int x0,y0;
	int x1,y1;
	int bandno;/*子带序号*/
	j2kTileCoder_Precinct *precincts;
	int numbps;
	int stepSize;
};

/************************************************************************/
/* 分辨率信息定义                                                                      */
/************************************************************************/
class j2kTileCoder_Resolution
{
public:
	int x0,y0;
	int x1,y1;
	int precinctWidth;/*此分辨率下水平上有多少个分区 */
	int precinctHeight;/*此分辨率下垂直上有多少个分区 */
	int numBands;/* 标记在此分辨率下有多少个子带 */
	j2kTileCoder_Band bands[3];
};

/************************************************************************/
/* tile分量信息定义                                                                      */
/************************************************************************/
class j2kTileCoder_Component
{
public:
	int x0,y0;
	int x1,y1;
	int numResolutions;
	j2kTileCoder_Resolution *resolutions;
	int *data;
};

class j2kTileCoder_Tile
{
public:
	int x0,y0;
	int x1,y1;
	int numComponents;
	j2kTileCoder_Component *comps;
	double distotile;
	double distolayer[100];
};

class j2kTileCoder_Image
{
public:
	int tw,th;
	j2kTileCoder_Tile *tiles;
};

class j2kTileCoder
{
public:
	CodeParam *cp;
	jp2Image *img;/* 当前图像引用 */
	int currentTileNo;

private:
	static j2kTileCoder_Tile *tile;
	static j2kTileCoder_Image image;/* 当前tile分量图像 */
	static j2kTileCoder_Component *tilec;
	static j2kTileCoder_Resolution *resolution;
	static j2kTileCoder_Band *band;
	static j2kTileCoder_Precinct *precinct;
	static j2kTileCoder_CodeBlock *codeblock;

public:
	j2kTileCoder(CodeParam *comp,jp2Image *image,int currentTile);

	void tcdMallocEncode();
	void tcdInitEncode();
	int tcdEncodeTilePxm(int tileno,unsigned char *dest,int len);
private:
	int dwtGetGain(int orient);
};

#endif