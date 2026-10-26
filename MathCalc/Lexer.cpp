#include "Lexer.hpp"

enum class State
{
	ReadingDigit,
	ReadingOperator,
	ReadingID,
	ReadingSpecChars,
	Undefined
};

DigitToken* Lexer::ReadDigit(std::string& expression, int& pos)
{
	std::string digit;

	// Вариант, когда введен один символ или лексер считывает один символ в конце строки.
	if (expression.size() == 1 || (pos + 1) == expression.size())
	{
		digit = expression[pos];
	}
	else
	{
		int j = pos + 1;

		for (; j < expression.size(); j++)
		{
			if (!std::isdigit(expression[j]))
			{
				break;
			}
		}

		digit = expression.substr(pos, (j - pos) + 1);

		// если цикл был прерван из-за j == expression.size(), то не нужно отступать на символ назад.
		if (j == expression.size())
		{
			pos = j;
		}
		else
		{
			pos = j - 1;
		}
	}

	return new DigitToken(std::stoi(digit));
}

IdToken* Lexer::ReadId(std::string& expression, int& pos)
{
	std::string idName;

	for (; pos < expression.size(); pos++)
	{
		if (isalpha(expression[pos]) || std::isdigit(expression[pos]) || expression[pos] == '_')
		{
			idName += expression[pos];
		}
		else
		{
			pos--;
			break;
		}
	}

	if (idName.size() == 0)
	{
		return nullptr;
	}

	IdToken* result = new IdToken();
	result->idName = idName;

	return result;
}

void Lexer::Scan(std::string& expression)
{
	if (expression.empty())
	{
		errorMessage_ = "String is empty";
		status_ = false;
		return;
	}

	int currentLine = 0;
	int currentColumn = 0;

	for (int i = 0; i < expression.size(); i++)
	{
		if (expression[i] == ' ')
		{
			currentColumn++;
			continue;
		}

		if (expression[i] == '\t')
		{
			currentColumn += 4;
			continue;
		}

		if (expression[i] == '\n')
		{
			currentLine++;

			currentColumn = 0;
			continue;
		}

		Token* token = nullptr;

		State state = State::Undefined;

		if (std::isdigit(expression[i]))
		{
			state = State::ReadingDigit;
		}
		else if (expression[i] == '+' || expression[i] == '-' || expression[i] == '*' || expression[i] == '/' || 
				 expression[i] == ',' || expression[i] == '=' || expression[i] == '>' || expression[i] == '<' || expression[i] == '.')
		{
			state = State::ReadingOperator;
		}
		else if (expression[i] == '(' || expression[i] == ')' || expression[i] == '{' || expression[i] == '}' || expression[i] == ';' || expression[i] == '&')
		{
			state = State::ReadingSpecChars;
		}
		else
		{
			state = State::ReadingID;
		}

		switch (state)
		{
		case State::ReadingDigit:
			token = ReadDigit(expression, i);
			break;
		case State::ReadingOperator:
		{
			OperatorToken* opToken = nullptr;

			if (expression[i] == '+')
			{
				opToken = new OperatorToken(OperatorTokenType::Plus);
			}
			else if (expression[i] == '-')
			{
				if (i + 1 != expression.size())
				{
					if (std::isdigit(expression[i + 1]))
					{
						DigitToken* tok = ReadDigit(expression, i);

						if (tok != nullptr)
						{
							tok->value = -tok->value;

							token = tok;
						}
					}
					else if (expression[i + 1] == ' ')
					{
						opToken = new OperatorToken(OperatorTokenType::Minus);
					}
					else
					{
						opToken = new OperatorToken(OperatorTokenType::UnaryMinus);
					}
				}
				else
				{
					opToken = new OperatorToken(OperatorTokenType::Minus);
				}
			}
			else if (expression[i] == '*')
			{
				opToken = new OperatorToken(OperatorTokenType::Multiply);
			}
			else if (expression[i] == '/')
			{
				opToken = new OperatorToken(OperatorTokenType::Divide);
			}
			else if (expression[i] == ',')
			{
				opToken = new OperatorToken(OperatorTokenType::Comma);
			}
			else if (expression[i] == '=')
			{
				opToken = new OperatorToken(OperatorTokenType::Assign);
			}
			else if (expression[i] == '>')
			{
				if (i + 1 != expression.size() && expression[i + 1] == '=')
				{
					opToken = new OperatorToken(OperatorTokenType::GreaterEqual);
					i++;
				}
				else
				{
					opToken = new OperatorToken(OperatorTokenType::Greater);
				}
			}
			else if (expression[i] == '<')
			{
				if (i + 1 != expression.size() && expression[i+1] == '=')
				{
					opToken = new OperatorToken(OperatorTokenType::LessEqual);
					i++;
				}
				else
				{
					opToken = new OperatorToken(OperatorTokenType::Less);
				}
			}
			else if (expression[i] == '.')
			{
				opToken = new OperatorToken(OperatorTokenType::Dot);
			}

			if (opToken != nullptr)
			{
				token = opToken;
			}
		}
		break;
		case State::ReadingSpecChars:
		{
			if (expression[i] == '(')
			{
				token = new OpenBracketToken();
			}
			else if (expression[i] == ')')
			{
				token = new CloseBracketToken();
			}
			else if (expression[i] == '{')
			{
				token = new OpenBraceToken();
			}
			else if (expression[i] == '}')
			{
				token = new CloseBraceToken();
			}
			else if (expression[i] == ';')
			{
				token = new SemicolonToken();
			}
			else if (expression[i] == '&')
			{
				token = new AmpersandToken();
			}
		}
		break;
		case State::ReadingID:
		{
			IdToken* temp = ReadId(expression, i);

			if (temp == nullptr)
			{
				errorMessage_ += std::string("Undefinded symbol: \"") + expression[i] + std::string("\" in position ") + std::to_string(i + 1) + "\n";
				status_ = false;
				continue;
			}

			if (keywords.contains(temp->idName))
			{
				KeywordToken* tok = new KeywordToken();

				tok->type = keywords[temp->idName];

				delete temp;

				token = tok;
			}
			else
			{
				token = temp;
			}
		}
		break;
		case State::Undefined:
			errorMessage_ += std::string("Undefinded symbol: \"") + expression[i] + std::string("\" in position ") + std::to_string(i + 1) + "\n";
			status_ = false;
			continue;
		break;
		}

		if (token != nullptr)
		{
			if (token->GetType() == TokenType::Id)
			{
				token->column = currentColumn;
				token->location.column = currentColumn;
				IdToken* tok = token->AsToken<IdToken>();

				currentColumn += tok->idName.size();
			}
			else if (token->GetType() == TokenType::Keyword)
			{
				token->column = currentColumn;
				token->location.column = currentColumn;

				KeywordToken* tok = token->AsToken<KeywordToken>();

				currentColumn += KeywordTypeToString(tok->type).size();
			}
			else
			{
				token->column = currentColumn;
				token->location.column = currentColumn;
				currentColumn++;
			}

			token->line = currentLine;
			token->location.line = currentLine;

			tokens_.push_back(token);
		}
	}
}
