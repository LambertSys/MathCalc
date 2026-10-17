#pragma once

#include <vector>
#include <string>

class Source
{
private:
	std::vector<std::string> lines_;

	std::string filePath_;

public:
	void SplitToLines(std::string& code);

	void SetPath(std::string& path);
	std::string GetPath();

	bool IsLineExist(int line);

	std::string& GetLine(int line);

	std::vector<std::string>& GetLines();
};

Source& GetFileSource();