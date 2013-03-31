/************************************************************************/
/* 用来存放编码和解码通用参数给所有的tiles                                                                     */
/************************************************************************/

#ifndef TCP
#define TCP
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

	TileCompCodeParam *tccps;
	PPT *ppt;
	POC *pocs;
};
#endif