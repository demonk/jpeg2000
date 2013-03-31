#include <iosfwd>
class File
{
private:
	std::streamoff m_size;//byte
	const char* m_fileName;
	int m_fileStatus;

public:
	File(const char* name);
	void setFileSize(std::streamoff size);
	std::streamoff getFileSize();
	const char* getFileName();

};
