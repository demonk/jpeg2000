#ifndef J2K_PACKET_ITERATOR
#define J2K_PACKET_ITERATOR
#include "j2kConst.h"
#include "POC.h"
#include "jp2Image.h"
#include "CodeParam.h"
#include "jpegMath.h"
#include <malloc.h>
#include <stdio.h>
// class piResolution
// {
// public:
// 	int precinctWidth;
// 	int precinctHeight;
// 	int pw;
// 	int ph;
// };

struct piResolution
{
	int precinctWidth;
	int precinctHeight;
	int pw;
	int ph;
};
// class piComponent
// {
// public:
// 	int dx;
// 	int dy;
// 	int numResolutions;
// 	piResolution *resolutions;
// 
// };
struct piComponent
{
	int dx;
	int dy;
	int numResolutions;
	struct piResolution *resolutions;
};

// class piIterator
// {
// public:
// 	short int *include ;/*给POC用的,如果包已经被用过则是清晰的, 对此项的位进行操作*/
// 	int stepLayer;
// 	int stepResolution;
// 	int stepComponent;
// 	int stepPrecinct;
// 	int compno,resno,precno,layno;/*用于标识包的*/
// 	int first;/*=0:是第一个包*/
// 	int numComponents;
// 	int tx0,ty0,tx1,ty1;
// 	int x,y,dx,dy;
// 	j2kPOC poc;
// 	piComponent *comps;
// };

struct piIterator
{
	short int *include ;/*给POC用的,如果包已经被用过则是清晰的, 对此项的位进行操作*/
	int stepLayer;
	int stepResolution;
	int stepComponent;
	int stepPrecinct;
	int compno,resno,precno,layno;/*用于标识包的*/
	int first;/*=0:是第一个包*/
	int numComponents;
	int tx0,ty0,tx1,ty1;
	int x,y,dx,dy;
	j2kPOC poc;
	struct piComponent *comps;
};

class PacketIterator
{

private:
	struct piIterator *pi;
	struct piIterator *piTwo;
public:
	PacketIterator();
	~PacketIterator();

	piIterator *piCreate(jp2Image *img,CodeParam *cp,int tileno);
	void piDestory();

	//piIterator *getIterator();

// 	int piEncodePacket(j2kTileCoder_Tile *tile,CodeParam *cp,unsigned char *dest,int len,int maxlayers,int tileno,j2kTierTwo *tier);
// 	int piNext(piIterator *pi);
// 	int piNext_LRCP(piIterator *pi);
// 	int piNext_RLCP(piIterator *pi);
// 	int piNext_RPCL(piIterator *pi);
// 	int piNext_PCRL(piIterator *pi);
// 	int piNext_CPRL(piIterator *pi);


};

#endif