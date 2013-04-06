#ifndef J2K_TIER_TWO
#define J2K_TIER_TWO
#include <malloc.h>
#include <string>
#include "j2kCustom.h"
#include "CodeParam.h"
#include "j2kTileCoder_Custom.h"
class piResolution
{
public:
	int precinctWidth;
	int precinctHeight;
	int pw;
	int ph;
};
class piComponent
{
public:
	int dx;
	int dy;
	int numResolutions;
	piResolution *resolutions;
};
class piIterator
{
public:
	short int *include ;/*给POC用的,如果包已经被用过则是清晰的*/
	int stepLayer;
	int stepResolution;
	int stepComponent;
	int stepPrecinct;
	int compno,resno,precno,layno;
	int first;/*=0:是第一个包*/
	int numComponents;
	int tx0,ty0,tx1,ty1;
	int x,y,dx,dy;
	int layerno;
	j2kPOC poc;
	piComponent *comps;
};
class j2kTierTwo
{
public:
	int tierTwoEncodePackets(jp2Image *img,CodeParam *cp,int tileno,j2kTileCoder_Tile *tile,int maxlayers,unsigned char *dest,int len);/* 编码一个tile的多个包去目标缓存中*/
	int tierTwoEncodePacket(j2kTileCoder_Tile *tile,TileCodeParam *tcp,int compno,int resno,int precno,int layerno,unsigned char *dest,int len,int tileno);/* 编码一个tile的包到目标缓存中 */

	void putNumPasses(int n);/* 信号delta Zil的可变长度代码 */
	void putCommaCode(int n);
	piIterator *piCreate(jp2Image *img,CodeParam *cp,int tileno);
	int piNext(piIterator *pi);
	int piNext_LRCP(piIterator *pi);
	int piNext_RLCP(piIterator *pi);
	int piNext_RPCL(piIterator *pi);
	int piNext_PCRL(piIterator *pi);
	int piNext_CPRL(piIterator *pi);
};
#endif