#ifndef J2K_TILE_CODER
#define J2K_TILE_CODER

#define S(i) data[numRes*(i)*2] //data与都是来自调用者那边的参数
#define D(i) data[numRes*(1+(i)*2)]
#define S_(i) ((i)<0?S(0):((i)>=sn?S(sn-1):S(i)))
#define D_(i) ((i)<0?D(0):((i)>=dn?D(dn-1):D(i)))
/* new */
#define SS_(i) ((i)<0?S(0):((i)>=dn?S(dn-1):S(i)))
#define DD_(i) ((i)<0?D(0):((i)>=sn?D(sn-1):D(i)))

//用8位定义方向
#define T1_SIG_NE 0x0001//North East
#define T1_SIG_SE 0x0002//South East
#define T1_SIG_SW 0x0004//South West
#define T1_SIG_NW 0x0008//North West
#define T1_SIG_N 0x0010//North
#define T1_SIG_E 0x0020//East
#define T1_SIG_S 0x0040//South
#define T1_SIG_W 0x0080//West

#define T1_SIG_OTH (T1_SIG_N|T1_SIG_NE|T1_SIG_E|T1_SIG_SE|T1_SIG_S|T1_SIG_SW|T1_SIG_W|T1_SIG_NW)//定义全方向8个领域
#define T1_SIG_PRIM (T1_SIG_N|T1_SIG_E|T1_SIG_S|T1_SIG_W)//只定义样本点上下左右点

#define T1_SGN_N 0x0100
#define T1_SGN_E 0x0200
#define T1_SGN_S 0x0400
#define T1_SGN_W 0x0800
#define T1_SGN (T1_SGN_N|T1_SGN_E|T1_SGN_S|T1_SGN_W)

//定义每个原语下有多少个上下文
#define T1_NUMCTXS_AGG 1
#define T1_NUMCTXS_ZC 9/* ZC */
#define T1_NUMCTXS_MAG 3 /* MR */
#define T1_NUMCTXS_SC 5 /* SC */
#define T1_NUMCTXS_UNI 1 /* RLC,UNIFORM上下文 */

#define T1_CTXNO_AGG 0 
#define T1_CTXNO_ZC (T1_CTXNO_AGG+T1_NUMCTXS_AGG)
#define T1_CTXNO_MAG (T1_CTXNO_ZC+T1_NUMCTXS_ZC)
#define T1_CTXNO_SC (T1_CTXNO_MAG+T1_NUMCTXS_MAG)//符号上下文标记起始=MAG起始+长度
#define T1_CTXNO_UNI (T1_CTXNO_SC+T1_NUMCTXS_SC)
#define T1_NUMCTXS (T1_CTXNO_UNI+T1_NUMCTXS_UNI)

#define T1_NMSEDEC_BITS 7
#define T1_NMSEDEC_FRACBITS (T1_NMSEDEC_BITS-1)

#define T1_SIG 0x1000
#define T1_REFINE 0x2000 //标记在magnitude refinement 中是否已经标记
#define T1_VISIT 0x4000 //标记是否已经访问

#define T1_TYPE_MQ 0
#define T1_TYPE_RAW 1

#define T1_MAXCBLKW 1024
#define T1_MAXCBLKH 1024


#include <stdlib.h>
#include "j2kCustom.h"
#include "CodeParam.h"
#include "TileCodeParam.h"
#include "jp2Image.h"
#include "StepSize.h"
#include "j2kTag.h"
#include "jpegMath.h"
#include "j2kMQC.h"
#include "j2kDWT.h"
class j2kTileCoder_Pass
{
private:
	int rate;
	double distortiondec;
	int term,len;
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
	tcd_seg_t segs[100];		/* segments informations */
	unsigned char data[8192];	/* Data */
	int numpassesinlayers;	/* number of passes in the layer */
	tcd_layer_t layers[100];	/* layer information */
	int totalpasses;		/* 记录编码次数 */
	j2kTileCoder_Pass passes[100];;	/* information about the passes */
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
private:
	/*9/7不可逆滤波器提升参数*/
	const int alpha=12993;//原-1.58,12993/8192=1.58,下同
	const int beta=434;
	const int gama=7233;
	const int sita=3633;
	const int K0=5038;
	const int K1=6659;//1/(6659/8192)=1.23,除以K等于乘以K的倒数

	int lastSizeOfB;
	int *widthData;


	/*---------TIER1--------*/
	//上下文
	int zcContextNo[1024];
	int scContextNo[256];
	int maContextNo[4096];
	int spbNo[256];

	int nmSeDecSig[1<<T1_NMSEDEC_BITS];
	int nmSeDecSig0[1<<T1_NMSEDEC_BITS];
	int nmSeDecRef[1<<T1_NMSEDEC_BITS];
	int nmSeDecRef0[1<<T1_NMSEDEC_BITS];

	int tierOneData[1024][1024];
	int tierOneFlags[1026][1026];

	/*----------MQC----------*/
	mqc_state_t *mqcCtxs[32];
	mqc_state_t **mqcCurCtx;/*当前的上下文信息*/
	unsigned int mqc_a;
	unsigned int mqc_c;
	unsigned int mqc_ct;
	unsigned int *mqc_bp;
	unsigned int *mqc_start;
private:

	j2kTileCoder_Image image;/* 当前tile分量图像 */
	
	jp2Image *img;/* 当前图像引用 */
	CodeParam *cp;

	j2kTileCoder_Tile *j2ktile;/*当前切片分量*/
	TileCodeParam *tcp;
	int currentTileNo;

	j2kTileCoder_Tile *tile;
	j2kTileCoder_Component *tilec;/*tile分量信息*/
	j2kTileCoder_Resolution *resolution;/*tile分辨率*/
	j2kTileCoder_Band *band;/*tile的子带*/
	j2kTileCoder_Precinct *precinct;/*tile分区*/
	j2kTileCoder_CodeBlock *codeblock;/*tile的代码块*/

public:
	j2kTileCoder(CodeParam *comp,jp2Image *image,int currentTile);

	void tcdMallocEncode();
	void tcdInitEncode();
	int tcdEncodeTilePxm(int tileno,unsigned char *dest,int len);/* 编码主函数  */
private:
	
	void multiCompTransform(TileCodeParam *tcp,j2kTileCoder_Tile *tile);
	void multiEncodeReal(j2kTileCoder_Tile *tile);/* RGB 转 YUV ,P292 */
	void multiEncode(j2kTileCoder_Tile *tile);/* RCT定义,p293*/

	void discreteWaveletTransform(TileCodeParam *tcp,j2kTileCoder_Tile *tile);
	void dwtEncodeReal(j2kTileCoder_Tile *tilec);/* Forward 9/7 wavelet transform in 2-D */
	void dwtEncode(j2kTileCoder_Tile *tilec);
	void dwtEncodeLowReal(int *data,int resWidth,int numRes,int resLowWidth,int iv);/* 9/7离散小波正变换 Forward Discrete Wavelet Transform*/
	void dwtEncodeLow(int *data,int resWidth,int numRes,int resLowWidth,int iv);/* 5/3 离散小波正变换 Forward  5/3 Wavelet Transform*/
	void dwtLazyTransform(int *data,int resWidth,int numRes,int resLowWidth,int iv);/* LAZY分割*/
	void dwtClean();

	void tierOneInitContext();/* 初始化上下文 */
	void tierEncodeCodeBlocks(j2kTileCoder_Tile *tile,TileCodeParam *tcp);
	void tierEncodeCodeBlock(j2kTileCoder_CodeBlock *codeBlock,j2kTileCoder_Tile *tile,int orient,int compno,int level,int isDWT,double stepSize,int codeBlockStyle,int numComps);
	int initCtxZC(int ctx,int bandno);
	int initCtxSC(int ctx);
	int initCtxMAG(int ctx);
	int initSPB(int ctx);
	int getCtxNoZC(int orient,int flag);
	int getCtxNoSC(int flag);
	int getSPB(int flag);

	void mqcResetStates();
	void mqcSetState(int ctxno,int msb,int prob);/* (上下文标记,上下文的MSB新state,标识上下文中新state中symbols的可能性) */
	void mqcInitEncode(unsigned char *data);
	void mqcSegMarkEncode();/*模式变化 ,SEGMARK*/
	void mqcByPassInitEncode();/*模式变化 ,BYPASS*/
	void mqcByPassEncode(int d);/* BYPASS 模式切换*/
	void mqcRestartInitEncode();/* 模式变化 ,RESTART */
	void mqcErtermEncode();/*模式变化,ENTREM*/
	void mqcFlush();/* 输出所有数据 */
	void mqcSetBits();/* 填充mqc_C*/
	void mqcSetCurCtx(int ctx);/*设置当前上下文标记*/
	int mqcNumBytes();/*获取自初始化以来所读/写的数据长度 */
	double getWmSeDec(int nmSeDec,int compno,int level,int orient,int bpsno,int isDWT,int stepSize,int numComps);
	void tierOneSignPass(int w,int h,int bpsno,int orient,int *nmSeDec ,char type,int cbstyle);/*重要性编码过程 */
	void tierOneSignPassStep(int *fp, int *dp, int orient, int bpno, int one,int *nmsedec, char type, int vsc);
	int tierOneGetNmSeDecSig(int x,int bitpos);
	void tierOneRefPass(int w,int h,int bpsno,int *nmSeDec,char type,int cbstyle);/*细化编码过程 */
	void tierOneRefPassStep(int *fp, int *dp, int orient, int bpno, int one,int *nmsedec, char type, int vsc);
	int tierOneGetNmSeDecRef(int x,int bitpos)
	void tierOneCleanPass(int w,int h,int bpsno,int orient,int *nmSeDec,int cbstyle);/*清理编码过程 */
	void tierOneClnPassStep(int *fp, int *dp, int orient, int bpsno, int one,int *nmsedec, int partial, int vsc);

	void tierOneUpdateFlags(int *fp,int s);

	void mqcEncode(int d);/*编码一个位symbol*/
	void mqcCodeMPS();/*对most probales symbol进行编码*/
	void mqcCodeLPS();/*对most least probales symbol进行编码 */
	void mqcRenormalize();/* 将mqc_A重正化,使其一起在0x8000~0x10000*/
	void mqcByteOut();/* 输出一个字节,如必要则进行位填充,在0xff之后,下一个字节一定得小于0x90*/
	int dwtGetGain(int orient);
};

#endif