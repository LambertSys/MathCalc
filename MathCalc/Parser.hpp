#pragma once

#include "AST.hpp"
#include "ASTBuilder.hpp"

class Lexer;

class Parser
{
private:
	std::string expression;

	bool status;

	std::string resultCode;

	ast::Context* context_;

	int counter;

private:
	std::string Traseval(ast::Node* node);

	void PrintErrors(Lexer& lexer, ast::Builder& builder);

public:
	Parser() : status(true), expression(), resultCode(), counter(0) {}
	Parser(std::string expression) : expression(expression), status(true), resultCode(), counter(0)
	{

	}

	void SetContext(ast::Context& context) { context_ = &context; }
	void SetExpression(std::string expression);

	void Parse();

	std::string GenerateCode();

	inline bool GetStatus()
	{
		return status;
	}
};