#include "SemaChecker.hpp"

#include "Report.hpp"
#include "Source.hpp"

#include <iostream>

using namespace ast;

DataType SemaChecker::CheckBinaryOperator(ast::Node* node)
{
	return DataType::Int32;
}

void SemaChecker::VisitFunctionCall(ast::FunctionCallNode* call)
{
	ast::FunctionDeclNode* funcNode = nullptr;

	SymbolTable* temp = currentTable_;

	bool argsIsWrong = false;

	while (temp != nullptr)
	{
		if (!temp->IsSymbolExist(call->Name()))
		{
			temp = temp->GetPrevTable();

			continue;
		}

		ast::Node* node = temp->GetSymbol(call->Name()).node;

		if (node->Type() != NodeType::FunctionDecl)
		{
			temp = temp->GetPrevTable();

			continue;
		}

		ast::FunctionDeclNode* func = node->AsNode<ast::FunctionDeclNode>();

		if (func->GetArgs().size() != call->GetArgs().size())
		{
			argsIsWrong = true;

			break;
		}

		// тут должны быть проверки типов, вывод типа из выражения и т.п. Поэтому скипаем expr ноды, где невозможно установить тип без анализа
		// Проверяем только ID ноды пока что
		for (int i = 0; i < func->GetArgs().size(); i++)
		{
			ast::FunctionArgNode* arg = func->GetArgs()[i];

			ast::Node* callArg = call->GetArgs()[i];

			SemanticTraseval(callArg);
		}

		funcNode = func;

		break;
	}

	if (argsIsWrong)
	{
		status_ = false;

		Report::PushError("'" + call->Name() + "' call: arguments don't match function signature", call->GetSourceLocation());

		return;
	}

	if (funcNode == nullptr)
	{
		status_ = false;

		Report::PushError(call->Name() + " function not declared in this Scope", call->GetSourceLocation());

		return;
	}

	call->SetCallFunction(funcNode);
}

void SemaChecker::VisitStmt(ast::StmtNode* stmt)
{
	if (stmt->GetNode()->Type() == ast::NodeType::Assign)
	{
		ast::VarNode* var = stmt->GetNode()->AsNode<ast::AssignNode>()->LeftNode()->AsNode<ast::VarNode>();

		varDecls_.insert({ var->Name(), var });

		SemanticTraseval(stmt->GetNode()->AsNode<ast::AssignNode>()->RightNode());
	}
}

void SemaChecker::VisitSeq(ast::SeqNode* seq)
{
	SemanticTraseval(seq->ExprNode());

	if (seq->NextNode() != nullptr)
	{
		VisitSeq(seq->NextNode());
	}
}

void SemaChecker::VisitFunctionDecl(ast::FunctionDeclNode* func)
{
	varDecls_.clear();

	for (auto arg : func->GetArgs())
	{
		varDecls_.insert({ arg->GetName(), arg->GetVariableNode() });
	}

	currentTable_ = &func->GetSymbolTable();

	VisitSeq(func->GetStartSeqNode());
}

void SemaChecker::SemanticTraseval(ast::Node* node)
{
	if (node == nullptr)
	{
		return;
	}

	switch (node->Type())
	{
	case ast::NodeType::FunctionDecl:
	{
		FunctionDeclNode* func = node->AsNode<FunctionDeclNode>();

		VisitFunctionDecl(func);

		//varDecls_.clear();

		//for (auto arg : func->GetArgs())
		//{
		//	varDecls_.insert({ arg->GetName(), arg->GetVariableNode() });
		//}

		//currentTable_ = &func->GetSymbolTable();

		//SemanticTraseval(func->GetStartSeqNode());
	}
	break;
	case ast::NodeType::Seq:
	{
		//ast::SeqNode* seq = node->AsNode<ast::SeqNode>();

		//SemanticTraseval(seq->ExprNode());

		//if (seq->NextNode() != nullptr)
		//{
		//	SemanticTraseval(seq->NextNode());
		//}
	}
	break;
	case ast::NodeType::Stmt:
	{
		ast::StmtNode* stmt = node->AsNode<ast::StmtNode>();

		VisitStmt(stmt);

		//if (stmt->GetNode()->Type() == ast::NodeType::Assign)
		//{
		//	ast::VarNode* var = stmt->GetNode()->AsNode<ast::AssignNode>()->LeftNode()->AsNode<ast::VarNode>();

		//	varDecls_.insert({ var->Name(), var });

		//	SemanticTraseval(stmt->GetNode()->AsNode<ast::AssignNode>()->RightNode());
		//}
	}
	break;
	case ast::NodeType::FunctionCall:
	{
		ast::FunctionCallNode* call = node->AsNode<ast::FunctionCallNode>();

		VisitFunctionCall(call);

		//ast::FunctionDeclNode* funcNode = nullptr;

		//SymbolTable* temp = currentTable_;

		//bool argsIsWrong = false;

		//while (temp != nullptr)
		//{
		//	if (!temp->IsSymbolExist(call->Name()))
		//	{
		//		temp = temp->GetPrevTable();

		//		continue;
		//	}

		//	ast::Node* node = temp->GetSymbol(call->Name()).node;

		//	if (node->Type() != NodeType::FunctionDecl)
		//	{
		//		temp = temp->GetPrevTable();

		//		continue;
		//	}

		//	ast::FunctionDeclNode* func = node->AsNode<ast::FunctionDeclNode>();

		//	if (func->GetArgs().size() != call->GetArgs().size())
		//	{
		//		argsIsWrong = true;

		//		break;
		//	}

		//	// тут должны быть проверки типов, вывод типа из выражения и т.п. Поэтому скипаем expr ноды, где невозможно установить тип без анализа
		//	// Проверяем только ID ноды пока что
		//	for (int i = 0; i < func->GetArgs().size(); i++)
		//	{
		//		ast::FunctionArgNode* arg = func->GetArgs()[i];

		//		ast::Node* callArg = call->GetArgs()[i];

		//		SemanticTraseval(callArg);
		//	}

		//	funcNode = func;

		//	break;
		//}

		//if (argsIsWrong)
		//{
		//	status_ = false;

		//	Report::PushError("'" + call->Name() + "' call: arguments don't match function signature", call->GetSourceLocation());

		//	return;
		//}

		//if (funcNode == nullptr)
		//{
		//	status_ = false;

		//	Report::PushError(call->Name() + " function not declared in this Scope", call->GetSourceLocation());

		//	return;
		//}

		//call->SetCallFunction(funcNode);
	}
	break;
	case ast::NodeType::BinaryOperator:
	{
		ast::BinaryOperatorNode* opNode = node->AsNode<ast::BinaryOperatorNode>();

		if (opNode->LeftOperand()->Type() == NodeType::Var)
		{
			ast::VarNode* id = opNode->LeftOperand()->AsNode<ast::VarNode>();
			
			if (!varDecls_.contains(id->Name()))
			{
				Report::PushError(id->Name() + " is undeclared in this Scope", id->GetSourceLocation());

				status_ = false;
			}
			else
			{
				ast::VarNode* varRef = varDecls_[id->Name()];

				delete id;

				opNode->SetLeftOperand(varRef);
				varRef->AddParent(opNode);
			}
		}
		else
		{
			SemanticTraseval(opNode->LeftOperand());
		}

		if (opNode->RightOperand()->Type() == NodeType::Var)
		{
			ast::VarNode* id = opNode->RightOperand()->AsNode<ast::VarNode>();

			if (!varDecls_.contains(id->Name()))
			{
				Report::PushError(id->Name() + " is undeclared in this Scope", id->GetSourceLocation());

				status_ = false;
			}
			else
			{
				ast::VarNode* varRef = varDecls_[id->Name()];

				delete id;

				opNode->SetRightOperand(varRef);
				varRef->AddParent(opNode);
			}
		}
		else
		{
			SemanticTraseval(opNode->RightOperand());
		}
	}
	break;
	case ast::NodeType::UnaryOperator:
	{
		ast::UnaryOperatorNode* opNode = node->AsNode<ast::UnaryOperatorNode>();

		if (opNode->Operand()->Type() == NodeType::Var)
		{
			ast::VarNode* id = opNode->Operand()->AsNode<ast::VarNode>();

			if (!varDecls_.contains(id->Name()))
			{
				Report::PushError(id->Name() + " is undeclared in this Scope", id->GetSourceLocation());

				status_ = false;

				return;
			}

			ast::VarNode* varRef = varDecls_[id->Name()];

			delete id;

			opNode->SetOperand(varRef);
			varRef->AddParent(opNode);
		}
		else
		{
			SemanticTraseval(opNode->Operand());
		}

	}
	break;
	case NodeType::Assign:
	{
		AssignNode* assign = node->AsNode<AssignNode>();

		ast::VarNode* id = assign->LeftNode()->AsNode<VarNode>();

		if (!varDecls_.contains(id->Name()))
		{
			Report::PushError(id->Name() + " is undeclared in this Scope", id->GetSourceLocation());

			status_ = false;

			return;
		}

		ast::VarNode* varRef = varDecls_[id->Name()];

		delete id;

		assign->SetLeftNode(varRef);

		SemanticTraseval(assign->RightNode());
	}
	break;
	}
}

void SemaChecker::PrintSemaErrors()
{
	for (auto& message : Report::GetMessages())
	{
		SourceLocation& loc = message.loc;

		std::cout << "line " + std::to_string(loc.line) + " | column " + std::to_string(loc.column) + std::string(": ") + message.message + "\n";

		std::string escape(loc.column, ' ');

		std::cout << "\t" << GetFileSource().GetLine(loc.line) << "\n";

		std::cout << "\t" << escape << "^\n";
	}
}

void SemaChecker::Analyze()
{
	for (auto func : context_->GetFunctionsDecls())
	{
		SemanticTraseval(func);
	}

	if (!status_)
	{
		std::cout << "Semantic Errors:\n";

		PrintSemaErrors();
	}
}
