#ifndef J2K_TIER_TWO
#define J2K_TIER_TWO
#include <malloc.h>
#include <string>
#include "j2kCustom.h"
#include "CodeParam.h"
#include "j2kPacketIterator.h"
#include "j2kTileCoder_Custom.h"


class j2kTierTwo
{
private:
	PacketIterator *packetIterator;
public:
	j2kTierTwo();
	~j2kTierTwo();

	int tierTwoEncodePackets(jp2Image *img,CodeParam *cp,int tileno,j2kTileCoder_Tile *tile,int maxlayers,unsigned char *dest,int len);/* 编码一个tile的多个包去目标缓存中*/
	int tierTwoEncodePacket(j2kTileCoder_Tile *tile,TileCodeParam *tcp,int compno,int resno,int precno,int layerno,unsigned char *dest,int len,int tileno);/* 编码一个tile的包到目标缓存中 */

	void putNumPasses(int n);/* 信号delta Zil的可变长度代码 */
	void putCommaCode(int n);
//	piIterator *piCreate(jp2Image *img,CodeParam *cp,int tileno);
	int piNext(piIterator *pi);
	int piNext_LRCP(piIterator *pi);
	int piNext_RLCP(piIterator *pi);
	int piNext_RPCL(piIterator *pi);
	int piNext_PCRL(piIterator *pi);
	int piNext_CPRL(piIterator *pi);
};
#endif