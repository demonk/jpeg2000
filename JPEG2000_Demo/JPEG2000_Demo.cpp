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

	wchar_t inFile[]=L"test.txt";
	const char *outFile="test-out.txt";

	Stream *inStream=new Stream(inFile,"rbe");

	if(inStream->open()>0)
		{
			char* buffer;
			if(inStream->read(buffer))
			{
				Logger::debug(buffer);
				inStream->write(buffer,strlen(buffer),false,outFile,"wb");
			}
			
	}
	else
		Logger::error("error");

	system("pause");
    return 0;
}
