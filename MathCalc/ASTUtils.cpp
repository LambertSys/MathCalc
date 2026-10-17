#include "ASTUtils.hpp"

#include <iostream>

namespace ast
{
	void PrintFunctionSymbols(FunctionDeclNode* node)
	{
		for (auto& symbol : node->GetSymbolTable().GetSymbols())
		{
			Node* n = symbol.node;

			std::string type = "";

			if (n->Type() == NodeType::FunctionArg)
			{
				type = "Function Argument";
			}
			else if (n->Type() == NodeType::Stmt)
			{
				type = "Variable";
			}

			std::cout << "\tSymbol type: " + type + "\n";
			std::cout << "\tSymbol name " + symbol.name + "\n";
		}
	}

	void PrintSymbolTable(SymbolTable& table)
	{
		for (auto& elem : table.GetSymbols())
		{
			Node* node = elem.node;

			switch (node->Type())
			{
			case NodeType::FunctionDecl:
			{
				std::cout << "Score of function " + node->AsNode<FunctionDeclNode>()->GetName() + "\n";

				PrintFunctionSymbols(node->AsNode<FunctionDeclNode>());

				std::cout << "\n";
			}
			break;
			}
		}
	}

	std::optional<DataType> GetTypeFromKeyword(KeywordToken* tok)
	{
		switch (tok->type)
		{
		case KeywordType::Int:
			return DataType::Int32;
		case KeywordType::UInt:
			return DataType::UInt32;
			break;
		case KeywordType::Bool:
			return DataType::Bool;
			break;
		case KeywordType::Void:
			return DataType::Void;
		}

		return std::optional<DataType>();
	}
}

