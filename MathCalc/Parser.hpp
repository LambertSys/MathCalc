#pragma once

#include "AST.hpp"
#include "ASTBuilder.hpp"

class Lexer;

class Parser
{
private:
	std::string expression_;

	bool status_ = true;

	std::string resultCode;

	ast::Context* context_ = nullptr;

	int counter_ = 0;

private:
	std::string Traseval(ast::Node* node);

	void PrintErrors(Lexer& lexer, ast::Builder& builder);

public:
	Parser() = default;
	Parser(std::string expression) : expression_(expression)
	{

	}

	void SetContext(ast::Context& context) { context_ = &context; }
	void SetExpression(std::string expression);

	void Parse();

	std::string GenerateCode();

	bool GetStatus()
	{
		return status_;
	}
};