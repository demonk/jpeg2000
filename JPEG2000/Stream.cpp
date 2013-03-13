
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
			openmode |= STREAM_WRITE ;//| STREAM_CREATE;
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

Stream::Stream()
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
	this->stream=NULL;
	this->img_pos=0;

}
Stream::~Stream()
{

}

void Stream::setStream(std::fstream *in)
{
	this->stream=in;
}

std::fstream* Stream::getStream()
{
	return this->stream;
}

void Stream::setFile(File *file)
{
	this->file=file;
}

File* Stream::getFile()
{
	return this->file;
}

int Stream::close()
{
	this->getStream()->close();
	delete this->stream;
	delete this->file;

	return 1;
}

int Stream::open(const char* fileName,const char *mode)
{
	this->setFile(new File(fileName));
	this->m_flags=this->CalMode(mode);

	this->stream=new std::fstream;
	this->stream->open(this->getFile()->getFileName(),this->m_flags);
	if(!this->stream->good())
	{
		Logger::error("open file failed");
		return 0;
	}
	if(this->m_flags&std::ios::in){
		//只是读需要
		this->getFile()->setFileSize(this->stream->tellg());
		this->stream->seekg(0,std::ios::beg);
		this->img_pos=0;
	}
	return 1;
}

int Stream::read(char *&buffer)
{
	if(this->stream==NULL)
	{
		Logger::error("STREAM_ERROR");
		return STREAM_ERROR;
	}
	if(this->stream->eof())
	{
		Logger::error("STREAM_EOF");
		return STREAM_EOF;
	}

	int i_remain;

	std::streamoff leftSize=this->file->getFileSize()-this->img_pos;
	i_remain=leftSize;

	if(leftSize>=STREAM_BUFFER_SIZE)
	{
		leftSize=STREAM_BUFFER_SIZE;
	}

	buffer=new char[leftSize];
	this->stream->read(buffer,leftSize);
	this->img_pos+=leftSize;
	return leftSize;
}

int Stream::write(const char *data,long size)
{
	/*
	std::ofstream *out=new std::ofstream;
	if(app)
	{
	out->open(file,CalMode(mode)|std::ios::ate);
	}else{
	out->open(file,CalMode(mode));
	}

	if(!out)
	{
	return -1;
	}*/
	if(this->stream==NULL)
	{
		Logger::error("STREAM_ERROR");
		return STREAM_ERROR;
	}

	this->stream->write(data,size);

	return 1;
}