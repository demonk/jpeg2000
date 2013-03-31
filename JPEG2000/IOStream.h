#ifndef IO_STREAM
#define  IO_STREAM

class IOStream{
public:
	static unsigned char *cio_start;
	static unsigned char *cio_end;
	static unsigned char *cio_bp;

public:
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

};
#endif