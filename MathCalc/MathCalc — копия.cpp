#include <iostream>
#include <string>
#include <vector>

enum class TokenType
{
	Digit,
	Operator,
	OpenBracket,
	CloseBracket
};

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
	default:
		return "Unknown";
	}
}

struct Token
{
	TokenType type;
	void* data;
	int column;
};

char GetCharValue(Token& token)
{
	if (token.type != TokenType::Operator)
	{
		return ' ';
	}

	return *static_cast<char*>(token.data);
}

class Lexer
{
public:
	enum class LexerStatus
	{
		Successfull,
		Error
	};

private:
	std::vector<Token> tokens;
	std::string errorMessage;

	LexerStatus status;

public:

	void Scan(std::string& expression)
	{
		if (expression.empty())
		{
			errorMessage = "String is empty";
			status = LexerStatus::Error;
			return;
		}

		for (int i = 0; i < expression.size(); i++)
		{
			Token token{};

			if (expression[i] == ' ')
			{
				continue;
			}

			if (std::isdigit(expression[i]))
			{
				std::string digit;

				// обход случаев, когда введен один символ или лексер считывает один символ в конце строки.
				if (expression.size() == 1 || (i + 1) == expression.size())
				{
					digit = expression[i];
				}
				else
				{
					int j = i + 1;

					for (; j < expression.size(); j++)
					{
						if (!std::isdigit(expression[j]))
						{
							break;
						}
					}

					digit = expression.substr(i, (j - i) + 1);

					// если цикл был прерван из-за j == expression.size(), то не нужно отступать на символ назад.
					if (j == expression.size())
					{
						i = j;
					}
					else
					{
						i = j - 1;
					}
				}

				int* digitValue = new int();

				*digitValue = std::stoi(digit);

				token.type = TokenType::Digit;
				token.data = (void*)digitValue;
			}
			else if (expression[i] == '+')
			{
				token.type = TokenType::Operator;
				token.data = new char('+');
			}
			else if (expression[i] == '-')
			{
				token.type = TokenType::Operator;

				//if (i + 1 < expression.size() && std::isdigit(expression[i + 1]))
				//{

				//}
				token.data = new char('-');
			}
			else if (expression[i] == '*')
			{
				token.type = TokenType::Operator;
				token.data = new char('*');
			}
			else if (expression[i] == '/')
			{
				token.type = TokenType::Operator;
				token.data = new char('/');
			}
			else if (expression[i] == '(')
			{
				token.type = TokenType::OpenBracket;
			}
			else if (expression[i] == ')')
			{
				token.type = TokenType::CloseBracket;
			}
			else
			{
				errorMessage += std::string("Undefinded symbol: \"") + expression[i] + std::string("\" in position ") + std::to_string(i + 1) + "\n";
				status = LexerStatus::Error;
				continue;
			}

			token.column = i + 1;
			tokens.push_back(token);
		}

		if (status != LexerStatus::Error)
		{
			status = LexerStatus::Successfull;
		}
	}

	LexerStatus GetStatus()
	{
		return status;
	}

	std::string GetErrorMessage()
	{
		return errorMessage;
	}

	std::vector<Token> GetTokensList()
	{
		return tokens;
	}
};

namespace AST
{
	enum class NodeType
	{
		Value,
		Operator
	};

	enum class OperatorType
	{
		Plus,
		Minus,
		Multiply,
		Divide,
		Undefinded
	};

	struct Node
	{
		NodeType type;
		OperatorType opType = OperatorType::Undefinded;
		void* data;

		Node* left;
		Node* right;
		Node* parent;
	};

	class Tree
	{
	private:
		Node* rootNode;

	public:
		void SetRootNode(Node* node) { rootNode = node; }
		Node* GetRootNode() { return rootNode; }
	};

	class Builder
	{
	private:
		bool status;
		std::string errorMessage;
		Tree tree;
		std::vector<Token> tokens;

		Node* GetDigit(int& tokenIndex)
		{
			if (tokenIndex == tokens.size())
			{
				return nullptr;
			}

			if (tokens[tokenIndex].type == TokenType::OpenBracket)
			{
				tokenIndex++;

				Node* expr = GetExpr(tokenIndex);

				if (tokenIndex == tokens.size() || tokens[tokenIndex].type != TokenType::CloseBracket)
				{
					errorMessage += "Close backet is missed\n";
					return nullptr;
				}

				tokenIndex++;

				return expr;
			}

			Node* digit = new Node();

			digit->type = NodeType::Value;
			digit->data = tokens[tokenIndex].data;

			tokenIndex++;

			return digit;
		}

		Node* GetTerm(int& tokenIndex)
		{
			Node* digit = GetDigit(tokenIndex);

			if (digit == nullptr)
			{
				errorMessage += ("expected digit in position " + std::to_string(tokenIndex) + "\n");
				return nullptr;
			}

			if (tokenIndex == tokens.size())
			{
				return digit;
			}

			Node* term;

			if (tokens[tokenIndex].type == TokenType::Operator)
			{
				term = new Node();

				OperatorType type;

				switch (GetCharValue(tokens[tokenIndex]))
				{
				case '*':
					tokenIndex++;
					type = OperatorType::Multiply;
					break;
				case '/':
					tokenIndex++;
					type = OperatorType::Divide;
					break;
				default:
					delete term;
					return digit;
				}

				term->type = NodeType::Operator;
				term->opType = type;
			}
			else
			{
				return digit;
			}

			Node* rightTerm = GetTerm(tokenIndex);

			if (rightTerm == nullptr)
			{
				delete term;
				return nullptr;
			}

			term->left = digit;
			term->right = rightTerm;

			return term;
		}

		Node* GetExpr(int& tokenIndex)
		{
			Node* expr;

			Node* termLeft = GetTerm(tokenIndex);

			if (termLeft == nullptr)
			{
				return nullptr;
			}

			if (tokenIndex == tokens.size())
			{
				return termLeft;
			}

			if (tokens[tokenIndex].type == TokenType::Operator)
			{
				expr = new Node();

				OperatorType type;

				switch (GetCharValue(tokens[tokenIndex]))
				{
				case '+':
					tokenIndex++;
					type = OperatorType::Plus;
					break;
				case '-':
					tokenIndex++;
					type = OperatorType::Minus;
					break;
				default:
					delete expr;
					return nullptr;
				}

				expr->type = NodeType::Operator;
				expr->opType = type;
			}
			else if (tokens[tokenIndex].type == TokenType::CloseBracket)
			{
				return termLeft;
			}
			else
			{
				errorMessage += "excepted + or + in postition " + std::to_string(tokens[tokenIndex].column) + std::string("\n");
				return nullptr;
			}

			Node* rightTerm = GetExpr(tokenIndex);

			if (rightTerm == nullptr)
			{
				delete expr;
				return nullptr;
			}

			expr->left = termLeft;
			expr->right = rightTerm;

			return expr;
		}

	public:
		void BuildTree(std::vector<Token>& tokens)
		{
			this->tokens = tokens;

			int tokenIndex = 0;

			Node* rootNode = GetExpr(tokenIndex);

			if (rootNode == nullptr)
			{
				status = false;

				return;
			}

			tree.SetRootNode(rootNode);

			status = true;
		}

		Tree GetTree() { return tree; }
		std::string GetMessage() { return errorMessage; }
		bool GetStatus() { return status; }
	};
}



class Processor
{
private:
	AST::Tree mathTree;

public:
	Processor(AST::Tree tree)
	{
		mathTree = tree;
	}

	int CalcOperator(AST::Node* node)
	{
		int leftResult;
		int rightResult;

		switch (node->opType)
		{
		case AST::OperatorType::Plus:
		{
			if (node->left->type == AST::NodeType::Value)
			{
				leftResult = *static_cast<int*>(node->left->data);
			}
			else
			{
				leftResult = CalcOperator(node->left);
			}

			if (node->right->type == AST::NodeType::Value)
			{
				rightResult = *static_cast<int*>(node->right->data);
			}
			else
			{
				rightResult = CalcOperator(node->right);
			}

			return leftResult + rightResult;
		}
		case AST::OperatorType::Minus:
		{
			if (node->left->type == AST::NodeType::Value)
			{
				leftResult = *static_cast<int*>(node->left->data);
			}
			else
			{
				leftResult = CalcOperator(node->left);
			}

			if (node->right->type == AST::NodeType::Value)
			{
				rightResult = *static_cast<int*>(node->right->data);
			}
			else
			{
				rightResult = CalcOperator(node->right);
			}

			return leftResult - rightResult;
		}
		case AST::OperatorType::Multiply:
		{
			if (node->left->type == AST::NodeType::Value)
			{
				leftResult = *static_cast<int*>(node->left->data);
			}
			else
			{
				leftResult = CalcOperator(node->left);
			}

			if (node->right->type == AST::NodeType::Value)
			{
				rightResult = *static_cast<int*>(node->right->data);
			}
			else
			{
				rightResult = CalcOperator(node->right);
			}

			return leftResult * rightResult;
		}
		case AST::OperatorType::Divide:
		{
			if (node->left->type == AST::NodeType::Value)
			{
				leftResult = *static_cast<int*>(node->left->data);
			}
			else
			{
				leftResult = CalcOperator(node->left);
			}

			if (node->right->type == AST::NodeType::Value)
			{
				rightResult = *static_cast<int*>(node->right->data);
			}
			else
			{
				rightResult = CalcOperator(node->right);
			}

			if (rightResult == 0)
			{
				std::cout << "Evaluation error: divide by zero\n";
				return -1;
			}

			return leftResult / rightResult;
		}
		case AST::OperatorType::Undefinded:
			return *static_cast<int*>(node->data);
		}

		return -1;
	}

	int Calc()
	{
		AST::Node* rootNode = mathTree.GetRootNode();

		if (rootNode->type == AST::NodeType::Value)
		{
			return *static_cast<int*>(rootNode->data);
		}

		return CalcOperator(rootNode);
	}
};

int main()
{
	std::string input = "1 + (2 * 4) - 6 + 2";

	Lexer lexer;

	lexer.Scan(input);

	if (lexer.GetStatus() == Lexer::LexerStatus::Error)
	{
		std::cout << "Lexer error:\n";
		std::cout << lexer.GetErrorMessage() << std::endl;
		return -1;
	}

	std::vector<Token> tokens = lexer.GetTokensList();

	for (int i = 0; i < tokens.size(); i++)
	{
		auto& item = tokens[i];

		std::cout << "Token index: " << i << std::endl << "Token type: " + TokenTypeToString(item.type) << std::endl << "Token column: " << std::to_string(item.column) << std::endl << "Token value: ";

		switch (item.type)
		{
		case TokenType::Digit:
			std::cout << *(static_cast<int*>(item.data));
			break;
		case TokenType::Operator:
			std::cout << *(static_cast<char*>(item.data));
			break;
		case TokenType::OpenBracket:
			std::cout << "(";
			break;
		case TokenType::CloseBracket:
			std::cout << ")";
			break;
		}

		std::cout << std::endl << std::endl;
	}

	AST::Builder builder;

	builder.BuildTree(tokens);

	if (!builder.GetStatus())
	{
		std::cout << "Parser error:\n";
		std::cout << builder.GetMessage() << std::endl;
		return -1;
	}

	std::cout << "parsing is successfull" << std::endl;

	AST::Tree tree = builder.GetTree();

	Processor proc(tree);

	int number = proc.Calc();

	std::cout << "The Ultimate Question of Life, the Universe, and Everything: " << number << std::endl;

	return 0;
}
