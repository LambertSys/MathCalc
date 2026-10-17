#pragma once

#include <map>
#include <vector>
#include <string>

namespace ast
{
	class Node;
}

struct Symbol
{
	ast::Node* node;
	std::string name;
	unsigned long long position;
};

class SymbolTable
{
private:
	std::vector<Symbol> symbols_;
	// имя | индекс  в symbols_
	std::map<std::string, int> names_;
	
	SymbolTable* prev_;

public:
	void AddName(std::string name, ast::Node* node)
	{
		names_.insert({ name, static_cast<int>(symbols_.size()) });

		symbols_.push_back(Symbol{ .node = node, .name = name, .position = symbols_.size() });
	}

	std::map<std::string, int>& GetMap() { return names_; }
	std::vector<Symbol>& GetSymbols() { return symbols_; }

	void SetPrevTable(SymbolTable* table) { prev_ = table; }

	SymbolTable* GetPrevTable() { return prev_; }

	bool IsSymbolExist(std::string name)
	{
		return names_.contains(name);
	}

	Symbol& GetSymbol(std::string name)
	{
		return symbols_[names_[name]];
	}
};
