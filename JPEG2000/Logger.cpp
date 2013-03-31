#include "Logger.h"

void Logger::debug(std::string msg)
{
	dispatch(DEBUG_COLOR::DEBUG,msg);
}

void Logger::warn(std::string msg)
{
	dispatch(DEBUG_COLOR::WARN,msg);
}

void Logger::error(std::string msg)
{
	dispatch(DEBUG_COLOR::ERROR,msg);
}

void Logger::dispatch(int level,std::string msg)
{
	switch(level)
	{
	case 0:
		//system("COLOR 02");
		break;
	case 1:
		//system("COLOR 06");
		break;
	case 2:
		//system("COLOR 04");
		break;
	default:
		//system("COLOR");
		break;
	}
	std::cout<<msg<<std::endl;
	system("COLOR");
}