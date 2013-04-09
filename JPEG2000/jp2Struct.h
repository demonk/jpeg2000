#ifndef JP2_STRUCT
#define JP2_STRUCT
#include <malloc.h>
#include "jp2Component.h"
#include "jp2Image.h"

//标准框标识
#define JPIP_JPIP 0x6a706970
#define JP2_JP   0x6a502020
#define JP2_FTYP 0x66747970
#define JP2_JP2H 0x6a703268
#define JP2_IHDR 0x69686472
#define JP2_COLR 0x636f6c72
#define JP2_JP2C 0x6a703263
#define JP2_URL  0x75726c20
#define JP2_DBTL 0x6474626c
#define JP2_BPCC 0x62706363
#define JP2_JP2  0x6a703220

/************************************************************************/
/* JP2 框结构                                                                     */
/************************************************************************/
class jp2Struct
{
public:  
  unsigned int width;
  unsigned int height;
  unsigned int numComponents;//图像分量总数
  unsigned int bpc;//bits per component框 ,必需,表示位深度
  unsigned int C;//image header 框中的CT表示压缩类型,惟一合法值是7,
  unsigned int UnkC;
  unsigned int IPR;//可选,知识产权框
  unsigned int meth;//header>image header框中的COLOR specification框中的M参数,M=1:表示彩色空间通过 ECS(枚举彩色空间)发送,M=2:表示彩色空间通过ICP发送 (ECS与ICP只有其一)
  unsigned int approx;
  unsigned int ECS;//ECS,枚举彩色空间
  unsigned int precedence;//优先级
  unsigned int brand;//file type框中的BR框,定义所采用的具体文件格式
  unsigned int minversion;//file type框中的MV框,定义商标最小版本号
  unsigned int numcl;//FILE TYPE 框 里指定文件所符合的标准兼容列表,此处是兼容的数目
  unsigned int *cl;//具体兼容的参数

  jp2Component *comps;
  jp2Image *image;

public:
	int jp2StructInit(jp2Image *img);
};
#endif