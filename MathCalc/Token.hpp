#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include "SourceLocation.hpp"

enum class TokenType
{
	Digit,
	Operator,
	OpenBracket, // (
	CloseBracket, // )
	Id,
	// ;
	Semicolon,
	Keyword,
	// {
	OpenBrace, 
	// }
	CloseBrace,
	// &
	Ampersand
};

enum class OperatorTokenType
{
	Plus, // +
	Minus, // -
	Multiply, // *
	Divide, // / 
	UnaryMinus, // -val
	Comma, // ,
	Dot, // .
	Assign, // =
	Greater, // >
	Less, // <
	Equal, // ==
	GreaterEqual, // >=
	LessEqual, // <=
};

enum class KeywordType
{
	Int = 0,
	UInt,
	Void,
	Bool,
	If,
	Else,
	Return,
	Struct,
	Extern
};

std::string KeywordTypeToString(KeywordType type);

extern std::unordered_map<std::string, KeywordType> keywords;

void InitKeywords();

std::string TokenTypeToString(TokenType enumValue);

struct Token
{
	SourceLocation location;

	int column = 0;
	int line = 0;

	virtual TokenType GetType() = 0;

	inline bool Is(TokenType type) { return type == GetType(); }

	template<typename TypeT>
	TypeT* AsToken()
	{
		return dynamic_cast<TypeT*>(this);
	}
};

struct DigitToken : public Token
{
	int value;

	DigitToken(int value) : value(value) {}

	TokenType GetType() override
	{
		return TokenType::Digit;
	}
};

struct OperatorToken : Token
{
	OperatorTokenType opType;

	OperatorToken(OperatorTokenType type) : opType(type) {}

	TokenType GetType() override
	{
		return TokenType::Operator;
	}
};

struct OpenBracketToken : Token
{
	TokenType GetType() override
	{
		return TokenType::OpenBracket;
	}
};

struct CloseBracketToken : Token
{
	TokenType GetType() override
	{
		return TokenType::CloseBracket;
	}
};

struct OpenBraceToken : Token
{
	TokenType GetType() override
	{
		return TokenType::OpenBrace;
	}
};

struct CloseBraceToken : Token
{
	TokenType GetType() override
	{
		return TokenType::CloseBrace;
	}
};

struct IdToken : Token
{
	std::string idName;

	TokenType GetType() override
	{
		return TokenType::Id;
	}
};

struct SemicolonToken : Token
{
	TokenType GetType() override
	{
		return TokenType::Semicolon;
	}
};

struct AmpersandToken : Token
{
	TokenType GetType() override
	{
		return TokenType::Ampersand;
	}
};

struct KeywordToken : Token
{
	KeywordType type;

	TokenType GetType() override
	{
		return TokenType::Keyword;
	}
};