#include "Source.hpp"

static Source source;

void Source::SplitToLines(std::string& code)
{
	int lastSepa = 0;

	for (int i = 0; i < code.size(); i++)
	{
		if (code[i] == '\n')
		{
			lines_.push_back(code.substr(lastSepa, i - lastSepa));

			// прибавляем 1, чтобы скипнуть \n, и он не мешал выводу
			lastSepa = i + 1;
		}
	}
}

void Source::SetPath(std::string& path)
{
	filePath_ = path;
}

std::string Source::GetPath()
{
	return filePath_;
}

bool Source::IsLineExist(int line)
{
	return lines_.size() > line;
}

std::string& Source::GetLine(int line)
{
	return lines_[line];
}

std::vector<std::string>& Source::GetLines()
{
	return lines_;
}

Source& GetFileSource()
{
	return source;
}
