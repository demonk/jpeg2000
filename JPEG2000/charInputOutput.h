#ifndef CHAR_IO_STREAM
#define  CHAR_IO_STREAM

namespace charInputOutput{
	void setPosition(int pos);
	int getLeftBytesLength();
	int getPosition();
	unsigned char* getCurrectChar();
	void init(unsigned char *bp,int len);
	void writeByte(unsigned char v);
	void writeBytes(unsigned int v,int n);
	unsigned char readByte();
	unsigned int readBytes(int n);
	void skipBytes(int n);
};
#endif