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

	char *inFile="fstream.doc";
	const char *outFile="fstream_out.doc";

	Stream *inStream=new Stream(inFile,"rbe");

	if(inStream->open()>0)
		{
			char* buffer;
			if(inStream->read(buffer))
			{
				inStream->write(buffer,inStream->getFile()->getFileSize(),false,outFile,"wb");
			}
			delete[] buffer;
			inStream->close();
	}
	else
		Logger::error("error");

	system("pause");
    return 0;
}
