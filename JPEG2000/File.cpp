#include "File.h"
using namespace JPEG2000;

File::File(const char* fileName)
{
	this->m_fileName=fileName;
}

void File::setFileSize(std::streamoff size)
{
	this->m_size=size;
}
std::streamoff File::getFileSize()
{
	return this->m_size;
}

const char* File::getFileName()
{
	return this->m_fileName;
}