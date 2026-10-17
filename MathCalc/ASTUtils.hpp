#pragma once

#include <optional>

#include "Token.hpp"

#include "AST.hpp"
#include "SymbolTable.hpp"

namespace ast
{
	void PrintFunctionSymbols(FunctionDeclNode* node);
	void PrintSymbolTable(SymbolTable& table);

	std::optional<DataType> GetTypeFromKeyword(KeywordToken* tok);
}