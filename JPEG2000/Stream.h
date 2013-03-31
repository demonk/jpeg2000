
#include <fstream>
#include <iostream>

#define STREAM_MAXPUTBACK 16
#define STREAM_READ 0X0001//0000 0000 0000 0001
#define STREAM_WRITE 0x0002//0000 0000 0000 0010
#define STREAM_APPEND 0x0004//0000 0000 0000 0100
#define STREAM_BINARY 0x0008//0000 0000 0000 1000
#define STREAM_CREATE 0x0010//0000 0000 0001 0000
#define STREAM_ATE 0x0020//0000 0000 0010 0000

#define STREAM_EOF -2
#define STREAM_ERROR -1

#define STREAM_BUFFER_SIZE 1024*10 //10K

class Stream
{
private:
	int m_openMode;
	int m_bufMode;
	int m_flags;
	int m_bufSize;
	int m_count;
	long m_rwCount;
	long m_rwLimit;

	unsigned char *ptr;
	unsigned char *p_bufBase;
	unsigned char *p_bufStart;
	unsigned char mTinyBuf[STREAM_MAXPUTBACK+1];

	std::fstream *stream;
	File* file;

	std::streamoff img_pos;
private:
	int CalMode(const char *s);//计算打开模式
protected:
	void setFile(File* file);

public:
	Stream();
	~Stream();

	void setStream(std::fstream *in);//需要指针,流不可复制
	std::fstream* getStream();
	File* getFile();

	int write(const char *data,long size);

	int open(const char* fileName,const char *mode);
	int read(char*& buffer);
	long seek();
	int close();
};
