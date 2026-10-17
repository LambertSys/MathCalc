#include "Parser.hpp"

#include "Lexer.hpp"
#include "LexerUtils.hpp"

#include "Report.hpp"

#include "Source.hpp"

#include "ASTDotPrinter.hpp"
#include "ASTUtils.hpp"

#include <fstream>
#include <iostream>

std::string Parser::Traseval(ast::Node* node)
{
	if (node == nullptr)
	{
		return "";
	}

	switch (node->Type())
	{
	case ast::NodeType::Seq:
	{
		Traseval(node->AsNode<ast::SeqNode>()->ExprNode());

		Traseval(node->AsNode<ast::SeqNode>()->NextNode());
	}
	break;
	case ast::NodeType::Stmt:
	{
		ast::StmtNode* stmt = node->AsNode<ast::StmtNode>();

		if (stmt->GetNode()->Type() == ast::NodeType::Var)
		{
			std::string name = Traseval(stmt->GetNode());

			resultCode += "Statement : " + name + "\n";
		}
		else
		{
			Traseval(stmt->GetNode());
		}
	}
	break;
	case ast::NodeType::FunctionDecl:
	{
		ast::FunctionDeclNode* decl = node->AsNode<ast::FunctionDeclNode>();

		std::string args = "";

		for (int i = 0; i < decl->GetArgs().size(); i++)
		{
			auto arg = decl->GetArgs()[i];

			args += arg->GetName();

			switch (arg->GetDataType())
			{
			case DataType::Int32:
				args += " : int32";
				break;
			case DataType::UInt32:
				args += " : uint32";
				break;
			}
			
			if (i + 1 != decl->GetArgs().size())
			{
				args += ", ";
			}
		}

		std::string retDataType = "";

		switch (decl->GetReturnType())
		{
		case DataType::Int32:
			retDataType = "int32";
			break;
		case DataType::UInt32:
			retDataType = "uint32";
			break;
		case DataType::Void:
			retDataType = "void";
			break;
		}

		resultCode += "Function Decl '" + decl->GetName() + "' (" + args + ") -> " + retDataType + "\n{\n";

		Traseval(decl->GetStartSeqNode());

		resultCode += "}\n\n";
	}
	break;
	case ast::NodeType::Assign:
	{
		ast::AssignNode* opNode = node->AsNode<ast::AssignNode>();

		std::string opName = "=";

		std::string idName = Traseval(opNode->LeftNode());
		std::string rightOp = Traseval(opNode->RightNode());

		resultCode += idName + " = " + rightOp + "\n";

		return idName;
	}
	case ast::NodeType::Value:
		return std::to_string(node->AsNode<ast::ValueNode>()->Value());
	case ast::NodeType::Var:
		return node->AsNode<ast::VarNode>()->Name();
	case ast::NodeType::Cast:
	{
		ast::CastNode* cast = node->AsNode<ast::CastNode>();

		std::string t = "t" + std::to_string(counter);

		std::string expr = Traseval(cast->GetExprNode());

		std::string typeTo = "";

		switch (cast->GetCastType())
		{
		case DataType::Int32:
			typeTo = "int32";
			break;
		case DataType::UInt32:
			typeTo = "uint32";
			break;
		}

		resultCode += t + " = cast " + typeTo + " " + expr + "\n";

		counter++;

		return t;
	}
	break;
	case ast::NodeType::UnaryOperator:
	{
		ast::UnaryOperatorNode* opNode = node->AsNode<ast::UnaryOperatorNode>();

		std::string opName = "-";

		std::string operand = Traseval(opNode->Operand());

		std::string t = "t" + std::to_string(counter);

		resultCode += t + " = " + opName + "(" + operand + ")\n";

		counter++;

		return t;
	}
	case ast::NodeType::BinaryOperator: 
	{
		ast::BinaryOperatorNode* opNode = node->AsNode<ast::BinaryOperatorNode>();

		std::string leftOp = Traseval(opNode->LeftOperand());
		std::string rightOp = Traseval(opNode->RightOperand());

		std::string opName = "";
		
		switch (opNode->OperatorType())
		{
		case ast::BinaryOperatorType::Plus:
			opName = "+";
			break;
		case ast::BinaryOperatorType::Minus:
			opName = "-";
			break;
		case ast::BinaryOperatorType::Multiply:
			opName = "*";
			break;
		case ast::BinaryOperatorType::Divide:
			opName = "/";
			break;
		case ast::BinaryOperatorType::Less:
			opName = "<";
			break;
		case ast::BinaryOperatorType::Greater:
			opName = ">";
			break;
		case ast::BinaryOperatorType::LessEqual:
			opName = "<=";
			break;
		case ast::BinaryOperatorType::GreaterEqual:
			opName = ">=";
			break;
		case ast::BinaryOperatorType::Equal:
			opName = "==";
			break;
		}

		std::string t = "t" + std::to_string(counter);

		resultCode += t + " = " + leftOp + " " + opName + " " + rightOp + "\n";
		
		counter++;

		return t;
	}
	case ast::NodeType::FunctionCall:
	{
		ast::FunctionCallNode* func = node->AsNode<ast::FunctionCallNode>();

		std::vector<std::string> argsNames;

		ast::ArgSeqNode* currentArg = func->RootArgNode();

		while (currentArg != nullptr)
		{
			argsNames.push_back(Traseval(currentArg->ExprNode()));

			currentArg = currentArg->NextArg();
		}

		std::string t = "t" + std::to_string(counter);

		resultCode += t + " = " + func->Name() + "(";

		for (int i = 0; i < argsNames.size(); i++)
		{
			if ((i + 1) == argsNames.size())
			{
				resultCode += argsNames[i];
			}
			else
			{
				resultCode += argsNames[i] + ", ";
			}
		}

		resultCode += ")\n";

		counter++;

		return t;
	}
	case ast::NodeType::ReturnStmt:
	{
		ast::ReturnStmtNode* ret = node->AsNode<ast::ReturnStmtNode>();

		if (ret->GetExprNode() != nullptr)
		{
			std::string t = Traseval(ret->GetExprNode());
			resultCode += "ret " + t + "\n";
		}
		else
		{
			resultCode += "ret " + std::string("\n");
		}
	}
	break;
	case ast::NodeType::IfStmt:
	{
		ast::IfStmtNode* stmt = node->AsNode<ast::IfStmtNode>();

		// TODO для if нужны метки, или разбиение на блоки
		// Не будет, будет сразу IR, из которого все генерируется хорошо
	}
	break;
	case ast::NodeType::ElseStmt:
	{
		// TODO для else нужны метки, или разбиение на блоки, и выставить связи на семантической фазе
				// Не будет, будет сразу IR, из которого все генерируется хорошо
	}
	break;
	default:
		throw std::exception("Not implemeted");
	}

	return "";
}

void Parser::PrintErrors(Lexer& lexer, ast::Builder& builder)
{
	status = false;

	std::cout << "Parser errors:\n";

	if (builder.GetErrorList().size() == 0)
	{
		std::cout << "Internal Parser Error" << std::endl;

		return;
	}

	PrintTokensList(lexer);

	for (auto& error : builder.GetErrorList())
	{
		std::cout << "line " + std::to_string(error.line) + " | column " + std::to_string(error.column) + std::string(": ") + error.message + "\n";

		Source& src = GetFileSource();

		std::cout << "\t" << src.GetLine(error.line) << "\n";

		std::string escape(error.column, ' ');

		std::cout << "\t" << escape << "^\n";
	}
}

void Parser::SetExpression(std::string expression)
{
	this->expression = expression;
}

void Parser::Parse()
{
	if (expression.empty())
	{
		status = false;

		std::cout << "Error: expression string is empty\n";

		return;
	}

	if (context_ == nullptr)
	{
		status = false;

		std::cout << "Error: context not setted";

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

	ast::Builder builder;

	builder.SetContext(context_);

	builder.Build(lexer.GetTokensList());

	ast::DotPrinter printer;
	printer.Print(*context_, "ast_pre_sema", false);

	std::cout << "Global Symbol Table:\n";

	ast::PrintSymbolTable(context_->GetGlobalTable());

	if (builder.GetErrorList().size() != 0)
	{
		PrintErrors(lexer, builder);

		status = false;
	}
}

std::string Parser::GenerateCode()
{
	for (auto func : context_->GetFunctionsDecls())
	{
		Traseval(func);

		counter = 0;
	}

	return resultCode;
}
