#include "Token.hpp"

std::unordered_map<std::string, KeywordType> keywords;

std::string KeywordTypeToString(KeywordType type)
{
	switch (type)
	{
	case KeywordType::Int:
		return "Int";
	case KeywordType::UInt:
		return "UInt";
	case KeywordType::Void:
		return "Void";
	case KeywordType::Bool:
		return "Bool";
	case KeywordType::If:
		return "If";
	case KeywordType::Else:
		return "else";
	case KeywordType::Return:
		return "return";
	case KeywordType::Extern:
		return "extern";
	}

	return std::string();
}

void InitKeywords()
{
	keywords.insert({ "int", KeywordType::Int });
	keywords.insert({ "uint", KeywordType::UInt });
	keywords.insert({ "void", KeywordType::Void });
	keywords.insert({ "bool", KeywordType::Bool });

	keywords.insert({ "if", KeywordType::If });
	keywords.insert({ "else", KeywordType::Else });

	keywords.insert({ "struct", KeywordType::Struct });

	keywords.insert({ "return", KeywordType::Return });

	keywords.insert({ "extern", KeywordType::Extern });
}

std::string TokenTypeToString(TokenType enumValue)
{
	switch (enumValue)
	{
	case TokenType::Digit:
		return "TokenType::Digit";
	case TokenType::Operator:
		return "TokenType::Operator";
	case TokenType::OpenBracket:
		return "TokenType::OpenBracket";
	case TokenType::CloseBracket:
		return "TokenType::CloseBracket";
	case TokenType::OpenBrace:
		return "TokenType::OpenBrace";
	case TokenType::CloseBrace:
		return "TokenType::CloseBrace";
	case TokenType::Id:
		return "TokenType::Id";
	case TokenType::Semicolon:
		return "TokenType::Semicolon";
	case TokenType::Keyword:
		return "TokenType::Keyword";
	default:
		return "Unknown";
	}
}
