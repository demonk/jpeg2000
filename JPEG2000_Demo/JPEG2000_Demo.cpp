// JPEG2000_Demo.cpp : main project file.
#include "stdafx.h"
#include "Logger.h"
#include "Image.h"
#include "Stream.h"
#include <iostream>
#include <fstream>
using namespace JPEG2000;

int main()
{
	//Image *image=new Image("set");

	char *inFile="3.jpg";
	const char *outFile="3_out.jpg";

	Stream *inStream=new Stream();
	Stream *outStream=new Stream();
	if(inStream->open(inFile,"rbe")>0)
	{
		char* buffer;
		int size;
		int i=0;
		outStream->open(outFile,"wba");
		std::fstream* stream=new std::fstream;

		while((size=inStream->read(buffer))>0)
		{
			outStream->write(buffer,size);
		}
		stream->close();

		delete[] buffer;
		inStream->close();
	}
	else
		Logger::error("error");

	//system("pause");
	return 0;
}
