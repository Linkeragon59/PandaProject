#pragma once

#include <string>
#include <fstream>
#include <iostream>

class File
{
public:
	File(std::string aPath);
	~File();
	std::string Read();
	void Write(std::string aString);

private:
	std::fstream myFile;
	std::string myPath;
};
