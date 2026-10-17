#pragma once

#include "SymbolTable.hpp"

#include "AST.hpp"

#include <optional>
#include <unordered_map>

class SemaChecker
{
private:
	bool status_ = true;

	ast::Context* context_ = nullptr;
	SymbolTable* currentTable_ = nullptr;

	ast::FunctionDeclNode* currentFunc_ = nullptr;

	ast::IfStmtNode* lastIfNode_ = nullptr;

	std::unordered_map<std::string, ast::VarNode*> varDecls_;

private:
	std::optional<DataType> VisitBinaryOperator(ast::Node* node);

	std::optional<DataType> VisitExpr(ast::Node* node);

	void VisitAssign(ast::AssignNode* assign);

	std::optional<DataType> VisitFunctionCall(ast::FunctionCallNode* call);

	void VisitElse(ast::ElseStmtNode* stmt);

	void VisitIf(ast::IfStmtNode* stmt);

	void VisitStmt(ast::StmtNode* stmt);

	void VisitReturn(ast::ReturnStmtNode* stmt);

	void VisitSeq(ast::SeqNode* seq);

	void VisitFunctionDecl(ast::FunctionDeclNode* func);

	void Traseval(ast::Node* node);

	void PrintSemaErrors();

public:
	SemaChecker(ast::Context& context) : context_(&context) {}

	void Analyze();

	bool GetStatus() { return status_; }
};