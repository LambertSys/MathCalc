#pragma once

#include "Token.hpp"

#include <vector>

class Lexer
{
private:
	std::vector<Token*> tokens_;

	std::string errorMessage_;
	bool status_ = true;

private:
	DigitToken* ReadDigit(std::string& expression, int& pos);
	IdToken* ReadId(std::string& expression, int& pos);

public:
	void Scan(std::string& expression);

	bool GetStatus() const
	{
		return status_;
	}

	std::string GetErrorMessage() const
	{
		return errorMessage_;
	}

	std::vector<Token*>& GetTokensList()
	{
		return tokens_;
	}
};