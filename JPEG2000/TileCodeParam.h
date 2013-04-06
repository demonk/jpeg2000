/************************************************************************/
/* 用来存放编码和解码通用参数给所有的tiles                                                                     */
/************************************************************************/

#ifndef J2K_TILE_CODE_PARAM
#define J2K_TILE_CODE_PARAM
#include "TileCompCodeParam.h"
#include "PPT.h"
#include "POC.h"
class TileCodeParam
{
public:
	int codingStyle;
	int progressionOrder;
	int numLayers;
	int numPocs;
	bool isMCT;
	int rates[100];

	float distoratio[100];/* 每一层的失真率 */
	int distoratioAlloc;
	int fixedQuality;

	TileCompCodeParam *tccps;
	j2kPPT *ppt;
	j2kPOC pocs[32];
	int pocUse;/*标记POC MARKER 是否已经使用*/
};
#endif