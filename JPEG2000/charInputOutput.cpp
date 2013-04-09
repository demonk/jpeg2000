#ifndef CHAR_IO_STREAM
#define  CHAR_IO_STREAM
#include "charInputOutput.h"

namespace charInputOutput{
	static unsigned char *cio_start;
	static unsigned char *cio_end;
	static unsigned char *cio_bp;

	static unsigned int size;

void setPosition(int pos)
{
	cio_bp=cio_start+pos;
}

int getLeftBytesLength()
{
	return cio_end-cio_bp;
}

int getPosition()
{
size=cio_bp-cio_start;
	return cio_bp-cio_start;
}

unsigned char* getCurrectChar()
{size=cio_bp-cio_start;
	return cio_bp;
}

void init(unsigned char *bp,int len)
{
	cio_start=bp;
	cio_end=bp+len;
	cio_bp=bp;
	size=cio_bp-cio_start;
}

void writeByte(unsigned char v)
{
	if(cio_bp<cio_end)
		*cio_bp++=v;
	size=cio_bp-cio_start;
}

void writeBytes(unsigned int v,int n)
{
	int i;
	for (i = n - 1; i >= 0; i--) {
		writeByte((unsigned char) ((v >> (i << 3)) & 0xff));
	}
	size=cio_bp-cio_start;
}
unsigned char readByte()
{
	if (cio_bp < cio_end)
		return *cio_bp++;
	size=cio_bp-cio_start;
	return 0;
}
unsigned int readBytes(int n)
{
	int i;
	unsigned int v;
	v = 0;
	for (i = n - 1; i >= 0; i--) {
		v += readByte() << (i << 3);
	}
	return v;
}

void skipBytes(int n)
{
	cio_bp += n;
	size=cio_bp-cio_start;
}

};
#endif