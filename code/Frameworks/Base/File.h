#pragma once

namespace File
{
	bool ReadAsBuffer(const std::string& aFilePath, std::vector<char>& anOutBuffer);
	bool ReadAsString(const std::string& aFilePath, std::string& anOutString);
}
