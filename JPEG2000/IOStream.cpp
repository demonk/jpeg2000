#include "IOStream.h"

void IOStream::setPosition(int pos)
{
	cio_bp=cio_start+pos;
}

int IOStream::getLeftBytesLength()
{
	return cio_end-cio_bp;
}

int IOStream::getPosition()
{
	return cio_bp-cio_start;
}

unsigned char* IOStream::getCurrectChar()
{
	return cio_bp;
}

void IOStream::init(unsigned char *bp,int len)
{
	cio_start=bp;
	cio_end=bp+len;
	cio_bp=bp;
}

void IOStream::writeByte(unsigned char v)
{
	if(cio_bp<cio_end)
		*cio_bp++=v;
}

void IOStream::writeBytes(unsigned int v,int n)
{
	int i;
	for (i = n - 1; i >= 0; i--) {
		writeByte((unsigned char) ((v >> (i << 3)) & 0xff));
	}
}
unsigned char IOStream::readByte()
{
	if (cio_bp < cio_end)
		return *cio_bp++;
	
}
unsigned int IOStream::readBytes(int n)
{
	int i;
	unsigned int v;
	v = 0;
	for (i = n - 1; i >= 0; i--) {
		v += readByte() << (i << 3);
	}
	return v;
}

void IOStream::skipBytes(int n)
{
	cio_bp += n;
}