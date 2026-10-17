#pragma once

#include "Token.hpp"

#include <vector>

class Lexer
{
private:
	std::vector<Token*> tokens;

	std::string errorMessage;
	bool status;

private:
	DigitToken* ReadDigit(std::string& expression, int& pos);
	IdToken* ReadId(std::string& expression, int& pos);

public:
	Lexer() : status(true), errorMessage(), tokens()
	{

	}

	void Scan(std::string& expression);

	inline bool GetStatus() const
	{
		return status;
	}

	inline std::string GetErrorMessage() const
	{
		return errorMessage;
	}

	inline std::vector<Token*>& GetTokensList()
	{
		return tokens;
	}
};