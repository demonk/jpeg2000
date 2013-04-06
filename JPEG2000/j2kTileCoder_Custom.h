#ifndef J2K_TILE_CODER_CUSTOM
#define J2K_TILE_CODER_CUSTOM
#include "j2kTag.h"
#include "j2kMQState.h"
class j2kTileCoder_Pass
{
public:
	int rate;
	double distortiondec;
	int term,len;
};

class j2kTileCoder_Layer
{
public:
	int numpasses;/*当前层编码的次数;*/
	int len;/* 信息长度*/
	int disto;
	unsigned char *data;
};
/************************************************************************/
/* 编码块信息定义                                                                      */
/************************************************************************/
class j2kTileCoder_CodeBlock
{
public:
	int x0,y0;
	int x1,y1;
	int numbps;

	int lastbp;			/* Add antonin : quantizbug1 */
	int numlenbits;
	int len;			/* length */
	int numpasses;		/* number of pass already done for the code-blocks */
	int numnewpasses;		/* number of pass added to the code-blocks */
	int numsegs;			/* number of segments */
	//tcd_seg_t segs[100];		/* segments informations */
	unsigned char data[8192];	/* Data */
	int numpassesinlayers;	/* number of passes in the layer */
	j2kTileCoder_Layer layers[100];	/* layer information */
	int totalpasses;		/* 记录编码次数 */
	j2kTileCoder_Pass passes[100];	/* information about the passes */
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
	int fixedQuality;
};

class j2kTileCoder_Tile
{
public:
	int x0,y0;
	int x1,y1;
	int numComponents;
	j2kTileCoder_Component *comps;
	int fixedQuality;
	double distotile;
	double distolayer[100];
};

class j2kTileCoder_Image
{
public:
	int tw,th;
	j2kTileCoder_Tile *tiles;
};

#endif