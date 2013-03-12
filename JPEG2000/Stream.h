
#include <fstream>
#include <streambuf>
#include <iostream>
#ifndef LOGGER
#include "Logger.h"
#endif


#define STREAM_MAXPUTBACK 16
#define STREAM_READ 0X0001//0000 0000 0000 0001
#define STREAM_WRITE 0x0002//0000 0000 0000 0010
#define STREAM_APPEND 0x0004//0000 0000 0000 0100
#define STREAM_BINARY 0x0008//0000 0000 0000 1000
#define STREAM_CREATE 0x0010//0000 0000 0001 0000
#define STREAM_ATE 0x0020//0000 0000 0010 0000

namespace JPEG2000
{

	class Stream
	{
	public:
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
		wchar_t *p_fileName;
		unsigned char mTinyBuf[STREAM_MAXPUTBACK+1];

		long m_size;
		std::fstream *stream;



	private:
		int CalMode(const char *s);
	protected:


	public:
		Stream(){}
		Stream(wchar_t* fileName,const char *mode);
		~Stream();

		void setInputStream(std::fstream *in);//需要指针,流不可复制
		std::fstream* getInputStream();

		int open();
		int read();
		int write();
		long seek();
		int close();
	};
}