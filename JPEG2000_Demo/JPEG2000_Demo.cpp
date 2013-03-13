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

	wchar_t fileName[]=L"test.txt";
	Stream *inStream=new Stream(fileName,"rbe");
	if(inStream->open()>0)
		{
			char* buffer;
			if(inStream->read(buffer))
				printf ("%s\n",buffer);
			
	}
	else
		Logger::error("error");

	system("pause");
    return 0;
}
