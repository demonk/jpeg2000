#include <string>
#include <iostream>
#define LOGGER

	class Logger
	{
	public:
		enum DEBUG_COLOR{
			DEBUG=0,
			WARN,
			ERROR
		};
		enum COLOR{
			BLACK=0,
			BLUE,
			GREEN,
			DARK_BLUE,
			RED,
			PURPLE,
			YELLOW,
			WHITE,
			GREY,
			LIGHT_BLUE,
			LIGHT_GREEN='A',
			LIGHT_GREEN2='B',
			LIGHT_RED='C',
			LIGHT_PURPLE='D',
			LIGHT_YELLOW='E',
			LIGHT_WHITE='F'
		};
	private:
		void static dispatch(int level,std::string msg);
	public:
		void static debug(std::string msg);
		void static warn(std::string msg);
		void static error(std::string msg);
	};
