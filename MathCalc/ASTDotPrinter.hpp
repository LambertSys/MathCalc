#pragma once

#include "AST.hpp"

#include <unordered_map>
#include <string>

namespace ast
{
	class DotPrinter
	{
	private:
		std::unordered_map<VarNode*, int> varNodes_;
		std::unordered_map<ElseStmtNode*, int> elseStmts_;

		int PrintVar(VarNode* node, std::string& buffer, std::string& relationsBuff, int& id);

		int PrintExpr(Node* node, std::string& buffer, std::string& relationsBuff, int& id);

		int PrintElseStmt(ElseStmtNode* stmt, std::string& buffer, std::string& relationsBuff, int& id);

		void PrintNode(Node* node, std::string& buffer, std::string& relationsBuff, int& id);

	public:
		void Print(ast::Context& context, std::string graphName, bool isCyclic);
	};
}