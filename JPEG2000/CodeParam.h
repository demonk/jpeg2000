#ifndef J2K_CODE_PARAM
#define J2K_CODE_PARAM
#include "PPM.h"
#include "TileCodeParam.h"

class CodeParam
	{
	public:
		int format;
		bool isIntermedFile;/* 是否分开几个文件储 */
		int imageType;
		int XTOsiz;
		int YTOsiz;
		int XTsiz;
		int YTsiz;

		int numTileWidth;
		int numTileHeight;

								char *comment;
		int *tileNo;/*tile的标识*/
		int tileNoSize;/* tile的总数*/

int disto_alloc;		/* Allocation by rate/distortion     */
  int fixed_alloc;		/* Allocation by fixed layer         */
  int fixed_quality;		/* add fixed_quality,PSNR值 */

  int tw;
		int th;
  PPM *ppm;
  TileCodeParam *tcps;


	};
#endif
