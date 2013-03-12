
#include "image.h"
using namespace JPEG2000;

Image::Image()
{
	this->mInputName="Random";

}

Image::Image(std::string name)
{
	this->mInputName=name;
}

void Image::decode()
{
	Logger::debug("==decode==");

}

void Image::encode()
{
	Logger::debug("==encode==");

}


std::string Image::__toString()
{
	return "InputFileName:"+this->mInputName;
}

Image::~Image()
{
	printf("I am release");
}