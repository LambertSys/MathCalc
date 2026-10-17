#include "LexerUtils.hpp"

#include <iostream>

void PrintTokensList(Lexer& lexer)
{
	std::cout << "Tokens list for Debug: \n";

	std::vector<Token*>& tokens = lexer.GetTokensList();

	for (int i = 0; i < tokens.size(); i++)
	{
		auto item = tokens[i];

		std::cout << "Token index: " << i << std::endl << "Token type: " + TokenTypeToString(item->GetType())
			<< std::endl << "Token line: " << std::to_string(item->line) << std::endl << "Token column: " << std::to_string(item->column)
			<< std::endl << "Token value: ";

		switch (item->GetType())
		{
		case TokenType::Digit:
			std::cout << item->AsToken<DigitToken>()->value;
			break;
		case TokenType::Operator:
		{
			std::string opChar = "";

			switch (item->AsToken<OperatorToken>()->opType)
			{
			case OperatorTokenType::Plus:
				opChar = "+ (Plus)";
				break;
			case OperatorTokenType::Minus:
				opChar = "- (Minus)";
				break;
			case OperatorTokenType::Multiply:
				opChar = "* (Multiply)";
				break;
			case OperatorTokenType::Divide:
				opChar = "/ (Divide)";
				break;
			case OperatorTokenType::UnaryMinus:
				opChar = "-value (Unary Minus)";
				break;
			case OperatorTokenType::Comma:
				opChar = ", (comma)";
				break;
			case OperatorTokenType::Dot:
				opChar = ". (Dot)";
				break;
			case OperatorTokenType::Greater:
				opChar = "> (Greater)";
				break;
			case OperatorTokenType::GreaterEqual:
				opChar = ">= (Greater Equal)";
				break;
			case OperatorTokenType::Less:
				opChar = "< (Less)";
				break;
			case OperatorTokenType::LessEqual:
				opChar = "<= (Less Equal)";
				break;
			case OperatorTokenType::Assign:
				opChar = "= (Assign)";
				break;
			}

			std::cout << opChar;
		}
		break;
		case TokenType::OpenBracket:
			std::cout << "(";
			break;
		case TokenType::CloseBracket:
			std::cout << ")";
			break;
		case TokenType::OpenBrace:
			std::cout << "{";
			break;
		case TokenType::CloseBrace:
			std::cout << "}";
			break;
		case TokenType::Id:
			std::cout << item->AsToken<IdToken>()->idName;
			break;
		case TokenType::Semicolon:
			std::cout << ";";
			break;
		case TokenType::Keyword:
			std::cout << KeywordTypeToString(item->AsToken<KeywordToken>()->type);
			break;
		}

		std::cout << std::endl << std::endl;
	}
}