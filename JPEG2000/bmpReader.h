#ifndef BMP_READER
#define BMP_READER
#include "jp2Image.h"
#include <iostream>
namespace bmpReader
{
	typedef struct {
		unsigned long int biSize;			/* Size of the structure in bytes */
		unsigned long int biWidth;		/* Width of the image in pixels */
		unsigned long int biHeight;		/* Heigth of the image in pixels */
		unsigned short int biPlanes;		/* 1 */
		unsigned short int biBitCount;		/* Number of color bits by pixels */
		unsigned long int biCompression;		/* Type of encoding 0: none 1: RLE8 2: RLE4 */
		unsigned long int biSizeImage;		/* Size of the image in bytes */
		unsigned long int biXpelsPerMeter;	/* Horizontal (X) resolution in pixels/meter */
		unsigned long int biYpelsPerMeter;	/* Vertical (Y) resolution in pixels/meter */
		unsigned long int biClrUsed;		/* Number of color used in the image (0: ALL) */
		unsigned long int biClrImportant;		/* Number of important color (0: ALL) */
	} BITMAPINFOHEADER_t;

	typedef struct {
		unsigned short bfType;			/* 'BM' for Bitmap (19776) */
		unsigned long int bfSize;			/* Size of the file        */
		unsigned short bfReserved1;		/* Reserved : 0            */
		unsigned short bfReserved2;		/* Reserved : 0            */
		unsigned long int bfOffBits;		/* Offset                  */
	} BITMAPFILEHEADER_t;

#ifndef _FILE_DEFINED
	struct _iobuf {
		char *_ptr;
		int   _cnt;
		char *_base;
		int   _flag;
		int   _file;
		int   _charbuf;
		int   _bufsiz;
		char *_tmpfname;
	};
	typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

	int bmpToImage(char *filename,jp2Image *img,int subSamplingX,int subSamplingY);

}
#endif