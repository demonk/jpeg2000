#ifndef JP2_WRITER
#define  JP2_WRITER
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

#include "j2kCustom.h"
#include "jp2Box.h"
#include "j2kCoder.h"

class jp2Writer
{
public:
	int encode(jp2Struct *jp2_struct,CodeParam *cp,char *buffer);

protected:
	void writeSignature();
	void writeFileType(jp2Struct *jp2_struct);
	void writeHeader(jp2Struct *jp2_struct);
	void writeIHDR(jp2Struct * jp2_struct);
	void writeBPCC(jp2Struct * jp2_struct);
	void writeColor(jp2Struct * jp2_struct);

	int writeContent(jp2Image *img,CodeParam *cp,char *buffer);
};
#endif