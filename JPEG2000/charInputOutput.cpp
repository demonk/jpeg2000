#ifndef CHAR_IO_STREAM
#define  CHAR_IO_STREAM
namespace charInputOutput{
	static unsigned char *cio_start;
	static unsigned char *cio_end;
	static unsigned char *cio_bp;

	static void setPosition(int pos);
	static int getLeftBytesLength();
	static int getPosition();
	static unsigned char* getCurrectChar();
	static void init(unsigned char *bp,int len);
	static void writeByte(unsigned char v);
	static void writeBytes(unsigned int v,int n);
	static unsigned char readByte();
	static unsigned int readBytes(int n);
	static void skipBytes(int n);


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
	return cio_bp-cio_start;
}

unsigned char* getCurrectChar()
{
	return cio_bp;
}

void init(unsigned char *bp,int len)
{
	cio_start=bp;
	cio_end=bp+len;
	cio_bp=bp;
}

void writeByte(unsigned char v)
{
	if(cio_bp<cio_end)
		*cio_bp++=v;
}

void writeBytes(unsigned int v,int n)
{
	int i;
	for (i = n - 1; i >= 0; i--) {
		writeByte((unsigned char) ((v >> (i << 3)) & 0xff));
	}
}
unsigned char readByte()
{
	if (cio_bp < cio_end)
		return *cio_bp++;

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
}

};
#endif