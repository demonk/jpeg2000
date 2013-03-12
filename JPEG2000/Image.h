#include <fstream>
#include <string>
#ifndef LOGGER
#include "Logger.h"
#endif
namespace JPEG2000
{
	class Image
	{
	private:
		std::string mInputName;
		std::string mOutputName;

	public:
		Image(std::string fileName);
		Image();
		~Image();

		void encode();
		void decode();
		std::string __toString();
	};
}