#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <optional>

enum class TokenType
{
	Digit,
	Operator,
	OpenBracket,
	CloseBracket
};

enum class OperatorType
{
	Plus,
	Minus,
	Multiply,
	Divide
};

static std::string TokenTypeToString(TokenType enumValue)
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

struct IToken
{
	int column;

	virtual TokenType GetType() = 0;
};

struct DigitToken : public IToken
{
	int value;

	DigitToken(int value) : value(value) {}

	TokenType GetType() override
	{
		return TokenType::Digit;
	}
};

struct OperatorToken : IToken
{
	OperatorType opType;

	OperatorToken(OperatorType type) : opType(type) {}

	TokenType GetType() override
	{
		return TokenType::Operator;
	}
};

struct OpenBracketToken : IToken
{
	TokenType GetType() override
	{
		return TokenType::OpenBracket;
	}
};

struct CloseBracketToken : IToken
{
	TokenType GetType() override
	{
		return TokenType::CloseBracket;
	}
};


class Lexer
{
private:
	std::vector<IToken*> tokens;

	std::string errorMessage;
	bool status;

public:
	Lexer() : status(true)
	{

	}

	void Scan(std::string& expression)
	{
		if (expression.empty())
		{
			errorMessage = "String is empty";
			status = false;
			return;
		}

		for (int i = 0; i < expression.size(); i++)
		{
			IToken* token;

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

				token = new DigitToken(std::stoi(digit));
			}
			else
			{
				OperatorToken* opToken = nullptr;

				if (expression[i] == '+')
				{
					opToken = new OperatorToken(OperatorType::Plus);
				}
				else if (expression[i] == '-')
				{
					opToken = new OperatorToken(OperatorType::Minus);
				}
				else if (expression[i] == '*')
				{
					opToken = new OperatorToken(OperatorType::Multiply);
				}
				else if (expression[i] == '/')
				{
					opToken = new OperatorToken(OperatorType::Divide);
				}

				if (opToken == nullptr)
				{
					if (expression[i] == '(')
					{
						token = new OpenBracketToken();
					}
					else if (expression[i] == ')')
					{
						token = new CloseBracketToken();
					}
					else
					{
						errorMessage += std::string("Undefinded symbol: \"") + expression[i] + std::string("\" in position ") + std::to_string(i + 1) + "\n";
						status = false;
						continue;
					}
				}
				else
				{
					token = opToken;
				}
			}

			token->column = i + 1;
			tokens.push_back(token);
		}
	}

	bool GetStatus() const
	{
		return status;
	}

	std::string GetErrorMessage() const
	{
		return errorMessage;
	}

	std::vector<IToken*>& GetTokensList()
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

	struct Node
	{
		Node* left;
		Node* right;
		Node* parent;

		Node() : left(nullptr), right(nullptr), parent(nullptr) {}

		virtual NodeType GetType() = 0;
	};

	struct ValueNode : public Node
	{
		int value;

		ValueNode(int value) : value(value) {}

		NodeType GetType() override
		{
			return NodeType::Value;
		}
	};

	struct OperatorNode : public Node
	{
		OperatorType opType;

		OperatorNode(OperatorType type) : opType(type) {}

		NodeType GetType() override
		{
			return NodeType::Operator;
		}
	};

	class Tree
	{
	private:
		Node* rootNode;

	public:
		void SetRootNode(Node* node) { rootNode = node; }
		Node* GetRootNode() { return rootNode; }
	};

	/*
	* Expr = Term + Expr | Term - Expr | Term
	* Term = Digit * Term | Digit / Term | Digit
	* Digit = <number> | (Expr)
	*/

	class Builder
	{
	private:
		bool status;
		std::string errorMessage;
		Tree tree;
		std::vector<IToken*> tokens;

		Node* GetDigit(int& tokenIndex)
		{
			if (tokenIndex == tokens.size())
			{
				return nullptr;
			}

			if (tokens[tokenIndex]->GetType() == TokenType::OpenBracket)
			{
				tokenIndex++;

				Node* expr = GetExpr(tokenIndex);

				if (tokenIndex == tokens.size() || tokens[tokenIndex]->GetType() != TokenType::CloseBracket)
				{
					errorMessage += "Close backet is missed\n";
					return nullptr;
				}

				tokenIndex++;

				return expr;
			}

			Node* digit = new ValueNode(static_cast<DigitToken*>(tokens[tokenIndex])->value);

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

			if (tokens[tokenIndex]->GetType() == TokenType::Operator)
			{
				switch (static_cast<OperatorToken*>(tokens[tokenIndex])->opType)
				{
				case OperatorType::Multiply:
					tokenIndex++;
					term = new OperatorNode(OperatorType::Multiply);
					break;
				case OperatorType::Divide:
					tokenIndex++;
					term = new OperatorNode(OperatorType::Divide);
					break;
				default:
					return digit;
				}
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

			Node* leftTerm = GetTerm(tokenIndex);

			if (leftTerm == nullptr)
			{
				return nullptr;
			}

			if (tokenIndex == tokens.size())
			{
				return leftTerm;
			}

			bool isMinus = false;

			if (tokens[tokenIndex]->GetType() == TokenType::Operator)
			{
				switch (static_cast<OperatorToken*>(tokens[tokenIndex])->opType)
				{
				case OperatorType::Plus:
					tokenIndex++;
					expr = new OperatorNode(OperatorType::Plus);
					break;
				case OperatorType::Minus:
					tokenIndex++;
					expr = new OperatorNode(OperatorType::Minus);
					isMinus = true;
					break;
				default:
					return nullptr;
				}
			}
			else if (tokens[tokenIndex]->GetType() == TokenType::CloseBracket)
			{
				return leftTerm;
			}
			else
			{
				errorMessage += "excepted + or + in postition " + std::to_string(tokens[tokenIndex]->column) + std::string("\n");
				return nullptr;
			}

			Node* rightExpr = GetExpr(tokenIndex);

			if (rightExpr == nullptr)
			{
				delete expr;
				// память освобождается неправильно.
				delete leftTerm;
				return nullptr;
			}

			expr->left = leftTerm;
			expr->right = rightExpr;

			return expr;
		}

	public:
		void BuildTree(std::vector<IToken*>& tokens)
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


static void PrintNode(AST::Node* node, std::string& buffer, std::string& relationsBuff, int& id)
{
	std::string prefix = "";

	if (node->GetType() == AST::NodeType::Value)
	{
		prefix = std::to_string(static_cast<AST::ValueNode*>(node)->value);
	}
	else
	{
		switch (static_cast<AST::OperatorNode*>(node)->opType)
		{
		case OperatorType::Plus:
			prefix = "+";
			break;
		case OperatorType::Minus:
			prefix = "-";
			break;
		case OperatorType::Multiply:
			prefix = "*";
			break;
		case OperatorType::Divide:
			prefix = "/";
			break;
		}
	}

	buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

	buffer += "];\n";

	int ourId = id;

	if (node->left != nullptr)
	{
		id++;

		relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

		PrintNode(node->left, buffer, relationsBuff, id);
	}

	if (node->right != nullptr)
	{
		id++;

		relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

		PrintNode(node->right, buffer, relationsBuff, id);
	}
}

static void PrintTree(AST::Tree& tree, std::string fileName)
{
	std::string buffer;
	std::string relationsBuff;

	buffer += "digraph ast {\n";

	int id = 0;

	PrintNode(tree.GetRootNode(), buffer, relationsBuff, id);

	buffer += "\n";

	buffer += relationsBuff;

	buffer += "}";

	std::ofstream file(fileName, std::ios::out);

	file.write(buffer.c_str(), buffer.size());

	file.close();
}


class Parser
{
private:
	std::string expression;
	bool status;
	AST::Tree result;

public:
	Parser() : status(true) {}
	Parser(std::string expression) : expression(expression), status(true)
	{

	}

	void SetExpression(std::string expression)
	{
		this->expression = expression;
	}

	void Parse()
	{
		if (expression.empty())
		{
			status = false;
			std::cout << "Error: expression string is empty\n";
			return;
		}

		Lexer lexer;

		lexer.Scan(expression);

		if (lexer.GetStatus() == false)
		{
			status = false;

			std::cout << "Lexer error:\n";
			std::cout << lexer.GetErrorMessage() << std::endl;

			return;
		}

		std::vector<IToken*> tokens = lexer.GetTokensList();

		for (int i = 0; i < tokens.size(); i++)
		{
			auto& item = tokens[i];

			std::cout << "Token index: " << i << std::endl << "Token type: " + TokenTypeToString(item->GetType()) << std::endl << "Token column: " << std::to_string(item->column) << std::endl << "Token value: ";

			switch (item->GetType())
			{
			case TokenType::Digit:
				std::cout << static_cast<DigitToken*>(item)->value;
				break;
			case TokenType::Operator:
			{
				std::string opChar = "";

				switch (static_cast<OperatorToken*>(item)->opType)
				{
				case OperatorType::Plus:
					opChar = "+ (Plus)";
					break;
				case OperatorType::Minus:
					opChar = "- (Minus)";
					break;
				case OperatorType::Multiply:
					opChar = "* (Multiply)";
					break;
				case OperatorType::Divide:
					opChar = "/ (Divide)";
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
			}

			std::cout << std::endl << std::endl;
		}


		AST::Builder builder;

		builder.BuildTree(lexer.GetTokensList());

		if (builder.GetStatus() == false)
		{
			status = false;

			std::cout << "Parser error:\n";
			std::cout << builder.GetMessage() << std::endl;

			return;
		}

		result = builder.GetTree();

		PrintTree(result, "ast.dot");
	}

	AST::Tree GetTree()
	{
		return result;
	}

	bool GetStatus()
	{
		return status;
	}
};

class MathEvaluator
{
private:
	AST::Tree mathTree;

public:
	MathEvaluator(AST::Tree tree)
	{
		mathTree = tree;
	}

	int CalcValue(AST::Node* node)
	{
		if (node->GetType() == AST::NodeType::Value)
		{
			return static_cast<AST::ValueNode*>(node)->value;
		}

		switch (static_cast<AST::OperatorNode*>(node)->opType)
		{
		case OperatorType::Plus:
		{
			return CalcValue(node->left) + CalcValue(node->right);
		}
		case OperatorType::Minus:
		{
			return CalcValue(node->left) - CalcValue(node->right);
		}
		case OperatorType::Multiply:
		{
			return CalcValue(node->left) * CalcValue(node->right);
		}
		case OperatorType::Divide:
		{
			int rightResult = CalcValue(node->right);

			if (rightResult == 0)
			{
				std::cout << "Evaluation error: divide by zero\n";
				return -1;
			}

			return CalcValue(node->left) / rightResult;
		}
		}

		return -1;
	}

	int Calc()
	{
		AST::Node* rootNode = mathTree.GetRootNode();

		if (rootNode->GetType() == AST::NodeType::Value)
		{
			return static_cast<AST::ValueNode*>(rootNode)->value;
		}

		return CalcValue(rootNode);
	}
};

int main()
{
	std::string input = "9 - 5 + 2";

	Parser parser;

	parser.SetExpression(input);

	parser.Parse();

	if (parser.GetStatus() == false)
	{
		std::cout << "Parser error" << std::endl;
		return -1;
	}

	AST::Tree tree = parser.GetTree();

	MathEvaluator proc(tree);

	//int number = proc.Calc();

	//std::cout << "The Ultimate Question of Life, the Universe, and Everything: " << number << std::endl;

	return 0;
}
