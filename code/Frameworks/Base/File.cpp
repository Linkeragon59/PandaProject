#include "File.h"

File::File(std::string aPath)
{
	myPath = aPath;
	myFile.open(myPath);
	if(myFile.fail())
	{
		std::cout << "Opening file " << aPath << " failed." << std::endl;
	}
}

File::~File()
{
	myFile.close();
}

std::string File::Read()
{
	std::string text;
	myFile >> text;
	return text;
}

void File::Write(std::string aString)
{
	myFile << aString;
}