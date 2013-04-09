#ifndef BIT_IO_STREAM
#define BIT_IO_STREAM

namespace bitInputOutput{

	void initEncoder(unsigned char *bp,int len);
	int getPosition();
	int writeByte();
	void writeBit(int b);
	void writeBits(int v,int n);
	int readByte();
	int readBit();
	int readBits(int n);
	int flush();
}
#endif