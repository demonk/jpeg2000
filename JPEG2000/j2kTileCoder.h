#ifndef J2K_TILE_CODER
#define J2K_TILE_CODER

#define S(i) data[numRes*(i)*2] //data与都是来自调用者那边的参数
#define D(i) data[numRes*(1+(i)*2)]
#define S_(i) ((i)<0?S(0):((i)>=sn?S(sn-1):S(i)))
#define D_(i) ((i)<0?D(0):((i)>=dn?D(dn-1):D(i)))
/* new */
#define SS_(i) ((i)<0?S(0):((i)>=dn?S(dn-1):S(i)))
#define DD_(i) ((i)<0?D(0):((i)>=sn?D(sn-1):D(i)))

#include <malloc.h>
#include <iostream>
#include "j2kTileCoder_Custom.h"
#include "StepSize.h"
#include "j2kTier1.h"
#include "j2kTier2.h"

class j2kTileCoder
{
private:
	/*9/7不可逆滤波器提升参数*/
	static const int alpha=12993;//原-1.58,12993/8192=1.58,下同
	static const int beta=434;
	static const int gama=7233;
	static const int sita=3633;
	static const int K0=5038;
	static const int K1=6659;//1/(6659/8192)=1.23,除以K等于乘以K的倒数

	static const int K=1;//应该为DOUBLE

	int lastSizeOfB;
	int *widthData;

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
	void dwtEncodeReal(j2kTileCoder_Component *tilec);/* Forward 9/7 wavelet transform in 2-D */
	void dwtEncode(j2kTileCoder_Component *tilec);
	void dwtEncodeLowReal(int *data,int resWidth,int numRes,int resLowWidth,int iv);/* 9/7离散小波正变换 Forward Discrete Wavelet Transform*/
	void dwtEncodeLow(int *data,int resWidth,int numRes,int resLowWidth,int iv);/* 5/3 离散小波正变换 Forward  5/3 Wavelet Transform*/
	void dwtLazyTransform(int *data,int resWidth,int numRes,int resLowWidth,int iv);/* LAZY分割*/
	void dwtClean();

	void makeLayerFixed(int layerno,int final);
	void makeLayer(int layerno,double thresh,int final);
	void rateAllocate(unsigned char *dest,int len,j2kTierTwo *tierTwo);
	void rateAllocateFixed();

	int dwtGetGain(int orient);
};

#endif