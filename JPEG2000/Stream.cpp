
#include "Stream.h"
#include <fcntl.h>
using namespace JPEG2000;

/* 从字符串中计算文件访问权限 */
int Stream::CalMode(const char *s)
{
	int openmode = 0;

	while (*s != '\0') {
		switch (*s) {
		case 'r':
			openmode |= STREAM_READ;
			break;
		case 'w':
			openmode |= STREAM_WRITE | STREAM_CREATE;
			break;
		case 'b':
			openmode |= STREAM_BINARY;
			break;
		case 'a':
			openmode |= STREAM_APPEND;
			break;
		case 'e':
			openmode|=STREAM_ATE;
			break;
		case '+':
			openmode |= STREAM_READ | STREAM_WRITE;
			break;
		default:
			break;
		}
		++s;
	}

	int openflags=0;
	if ((openmode & STREAM_READ) &&
		(openmode & STREAM_WRITE)) {
			openflags = std::ios::in|std::ios::out;
	} else if (openmode & STREAM_READ) {
		openflags =std::ios::in;
	} else if (openmode & STREAM_WRITE) {
		openflags =std::ios::out;
	} else {
		openflags = 0;
	}

	if (openmode & STREAM_APPEND) {
		openflags |= std::ios::app;
	}

	if(openmode&STREAM_ATE)
	{
		openflags|=std::ios::ate;
	}

	if (openmode & STREAM_BINARY) {
		openflags |= std::ios::binary;
	}
	if (openmode & STREAM_CREATE) {
		openflags |= std::ios::trunc;//create 
	}

	return openflags;
}

Stream::Stream(wchar_t* fileName,const char *mode)
{
	this->m_openMode=0;
	this->m_bufMode=0;
	this->m_bufSize=0;
	this->m_count=0;
	this->m_rwCount=0;
	this->m_rwLimit=0;
	this->ptr=0;
	this->p_bufBase=0;
	this->p_bufStart=0;
	this->m_flags=this->CalMode(mode);
	this->p_fileName=fileName;
	this->stream=NULL;
	this->m_size=0;
}

Stream::~Stream()
{

}

int Stream::open()
{
	//std::fstream inFile;
	this->stream=new std::fstream;
	this->stream->open(this->p_fileName,this->m_flags);
	//inFile.open("test.txt",std::ios::in|std::ios::out|std::ios::binary|std::ios::ate);
	if(!this->stream->good())
	{
		Logger::error("open file failed");
		return 0;
	}

	std::streamoff pos=this->stream->tellg();
	
	this->m_size=this->stream->tellg();
	std::cout<<this->m_size;
	this->stream->seekg(0,std::ios::beg);
	return 1;
}

int Stream::read()
{
	if(this->stream==NULL)
	{
		return -1;
	}

	char *buffer=new char[this->m_size];
	
	this->stream->read(buffer,this->m_size);
	Logger::debug(buffer);
	delete[] buffer;
	return 0;
}

void Stream::setInputStream(std::fstream *in)
{
	this->stream=in;
}

std::fstream* Stream::getInputStream()
{
	return this->stream;
}