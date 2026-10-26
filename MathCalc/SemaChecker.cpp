#include "SemaChecker.hpp"

#include "Report.hpp"
#include "Source.hpp"

#include "TypeUtils.h"

#include <iostream>

using namespace ast;

std::optional<DataType> SemaChecker::VisitBinaryOperator(ast::Node* node)
{
	return DataType::Int32;
}

std::optional<DataType> SemaChecker::VisitExpr(ast::Node* node)
{
	if (node == nullptr)
	{
		return std::nullopt;
	}

	switch (node->Type())
	{
	case ast::NodeType::FunctionCall:
	{
		ast::FunctionCallNode* call = node->AsNode<ast::FunctionCallNode>();

		auto ret = VisitFunctionCall(call);

		if (ret.has_value())
		{
			return ret.value();
		}
		else
		{
			return std::nullopt;
		}
	}
	break;
	case ast::NodeType::Value:
		return DataType::Int32; // TODO в будущем будут константы разных типов
	case ast::NodeType::Cast:
	{
		CastNode* cast = node->AsNode<CastNode>();

		VisitExpr(cast->GetExprNode());

		return cast->GetCastType();
	}
	case ast::NodeType::BinaryOperator:
	{
		ast::BinaryOperatorNode* opNode = node->AsNode<ast::BinaryOperatorNode>();

		DataType leftType = DataType::Max;
		DataType rightType = DataType::Max;

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

				leftType = varRef->GetDataType();
			}
		}
		else
		{
			auto ret = VisitExpr(opNode->LeftOperand());

			if (ret.has_value())
			{
				leftType = ret.value();
			}
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

				rightType = varRef->GetDataType();
			}
		}
		else
		{
			auto ret = VisitExpr(opNode->RightOperand());

			if (ret.has_value())
			{
				rightType = ret.value();
			}
		}

		if (opNode->OperatorType() == BinaryOperatorType::Greater || opNode->OperatorType() == BinaryOperatorType::Less || opNode->OperatorType() == BinaryOperatorType::Equal ||
			opNode->OperatorType() == BinaryOperatorType::GreaterEqual || opNode->OperatorType() == BinaryOperatorType::LessEqual)
		{
			opNode->SetSemaDataType(DataType::Bool);
		}

		// слева ошибка
		if (leftType == DataType::Max)
		{
			if (rightType != DataType::Max)
			{
				if (!(opNode->OperatorType() == BinaryOperatorType::Greater || opNode->OperatorType() == BinaryOperatorType::Less || opNode->OperatorType() == BinaryOperatorType::Equal ||
					opNode->OperatorType() == BinaryOperatorType::GreaterEqual || opNode->OperatorType() == BinaryOperatorType::LessEqual))
				{
					opNode->SetSemaDataType(rightType);
				}

				return rightType;
			}
			else
			{
				return std::nullopt;
			}
		}

		// справа ошибка
		if (rightType == DataType::Max)
		{
			if (opNode->OperatorType() == BinaryOperatorType::Greater || opNode->OperatorType() == BinaryOperatorType::Less || opNode->OperatorType() == BinaryOperatorType::Equal ||
				opNode->OperatorType() == BinaryOperatorType::GreaterEqual || opNode->OperatorType() == BinaryOperatorType::LessEqual)
			{
				opNode->SetSemaDataType(DataType::Bool);
			}
			else
			{
				opNode->SetSemaDataType(leftType);
			}

			return leftType;
		}

		// преобразование не требуется
		if (leftType == rightType)
		{
			if (opNode->OperatorType() == BinaryOperatorType::Greater || opNode->OperatorType() == BinaryOperatorType::Less || opNode->OperatorType() == BinaryOperatorType::Equal ||
				opNode->OperatorType() == BinaryOperatorType::GreaterEqual || opNode->OperatorType() == BinaryOperatorType::LessEqual)
			{
				opNode->SetSemaDataType(DataType::Bool);
			}
			else
			{
				opNode->SetSemaDataType(leftType);
			}

			return leftType;
		}

		CastNode* cast = new CastNode();

		if (GetRank(leftType) == GetRank(rightType))
		{
			if (IsUnsigned(leftType))
			{
				cast->SetCastType(leftType);
				cast->SetExprNode(opNode->RightOperand());

				opNode->SetRightOperand(cast);
				opNode->SetSemaDataType(leftType);
			}
			else
			{
				cast->SetCastType(rightType);
				cast->SetExprNode(opNode->LeftOperand());

				opNode->SetLeftOperand(cast);
				opNode->SetSemaDataType(rightType);
			}
		}
		else if(GetRank(leftType) > GetRank(rightType))
		{
			cast->SetCastType(leftType);
			cast->SetExprNode(opNode->RightOperand());

			opNode->SetRightOperand(cast);
			opNode->SetSemaDataType(leftType);
		}
		else if (GetRank(leftType) < GetRank(rightType))
		{
			cast->SetCastType(rightType);
			cast->SetExprNode(opNode->LeftOperand());

			opNode->SetLeftOperand(cast);
			opNode->SetSemaDataType(rightType);
		}

		// логические операторы
		if (opNode->OperatorType() == BinaryOperatorType::Greater || opNode->OperatorType() == BinaryOperatorType::Less || opNode->OperatorType() == BinaryOperatorType::Equal ||
			opNode->OperatorType() == BinaryOperatorType::GreaterEqual || opNode->OperatorType() == BinaryOperatorType::LessEqual)
		{
			opNode->SetSemaDataType(DataType::Bool);
		}
		
		return opNode->GetSemaDataType();
	}
	break;
	case ast::NodeType::UnaryOperator:
	{
		ast::UnaryOperatorNode* opNode = node->AsNode<ast::UnaryOperatorNode>();

		DataType retType = DataType::Max;

		if (opNode->Operand()->Type() == NodeType::Var)
		{
			ast::VarNode* id = opNode->Operand()->AsNode<ast::VarNode>();

			if (!varDecls_.contains(id->Name()))
			{
				Report::PushError(id->Name() + " is undeclared in this Scope", id->GetSourceLocation());

				status_ = false;

				return std::nullopt;
			}

			ast::VarNode* varRef = varDecls_[id->Name()];

			delete id;

			opNode->SetOperand(varRef);
			varRef->AddParent(opNode);

			retType = varRef->GetDataType();
		}
		else
		{
			auto ret = VisitExpr(opNode->Operand());

			if (ret.has_value())
			{
				retType = ret.value();
			}
			else
			{
				return std::nullopt;
			}
		}

		opNode->SetDataType(retType);

		return retType;
	}
	break;
	//case NodeType::Assign:
	//{
	//	AssignNode* assign = node->AsNode<AssignNode>();

	//	ast::VarNode* id = assign->LeftNode()->AsNode<VarNode>();

	//	if (!varDecls_.contains(id->Name()))
	//	{
	//		Report::PushError(id->Name() + " is undeclared in this Scope", id->GetSourceLocation());

	//		status_ = false;

	//		return;
	//	}

	//	ast::VarNode* varRef = varDecls_[id->Name()];

	//	delete id;

	//	assign->SetLeftNode(varRef);

	//	SemanticTraseval(assign->RightNode());
	//}
	//break;
	}
	return DataType();
}

void SemaChecker::VisitAssign(ast::AssignNode* assign)
{
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

	auto ret = VisitExpr(assign->RightNode());

	if (!ret.has_value())
	{
		return;
	}

	DataType rightType = ret.value();

	if (rightType != varRef->GetDataType())
	{
		CastNode* cast = new CastNode();

		cast->SetCastType(varRef->GetDataType());
		cast->SetExprNode(assign->RightNode());
		
		assign->SetRightNode(cast);
	}
}

std::optional<DataType> SemaChecker::VisitFunctionCall(ast::FunctionCallNode* call)
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

		for (int i = 0; i < func->GetArgs().size(); i++)
		{
			ast::FunctionArgNode* arg = func->GetArgs()[i];

			ast::Node* callArg = call->GetArgs()[i];

			auto ret = VisitExpr(callArg);

			if (!ret.has_value() || ret.value() == arg->GetDataType())
			{
				continue;
			}

			DataType leftType = arg->GetDataType();
			DataType rightType = ret.value();

			CastNode* cast = new CastNode();

			cast->SetCastType(leftType);
			cast->SetExprNode(callArg);

			call->GetArgs()[i] = cast;
		}

		funcNode = func;

		break;
	}

	if (argsIsWrong)
	{
		status_ = false;

		Report::PushError("'" + call->Name() + "' call: arguments don't match function signature", call->GetSourceLocation());

		return std::nullopt;
	}

	if (funcNode == nullptr)
	{
		status_ = false;

		Report::PushError(call->Name() + " function not declared in this Scope", call->GetSourceLocation());

		return std::nullopt;
	}

	call->SetCallFunction(funcNode);

	return funcNode->GetReturnType();
}

void SemaChecker::VisitElse(ast::ElseStmtNode* stmt)
{
	if (lastIfNode_ == nullptr)
	{
		Report::PushError("Else block without a paired if block", stmt->GetSourceLocation());

		return;
	}

	stmt->SetIfStmt(lastIfNode_);

	lastIfNode_ = nullptr;

	VisitSeq(stmt->GetStartSeq());
}

void SemaChecker::VisitIf(ast::IfStmtNode* stmt)
{
	lastIfNode_ = stmt;

	auto condType = VisitExpr(stmt->GetCond());

	if (condType.has_value() && condType.value() != DataType::Bool)
	{
		CastNode* cast = new CastNode();

		cast->SetCastType(DataType::Bool);
		cast->SetExprNode(stmt->GetCond());

		stmt->SetCond(cast);
	}

	VisitSeq(stmt->GetStartSeq());
}

void SemaChecker::VisitStmt(ast::StmtNode* stmt)
{
	if (stmt->GetNode()->Type() == ast::NodeType::Assign)
	{
		AssignNode* assign = stmt->GetNode()->AsNode<ast::AssignNode>();

		ast::VarNode* var = assign->LeftNode()->AsNode<ast::VarNode>();

		varDecls_.insert({ var->Name(), var });

		auto ret = VisitExpr(assign->RightNode());

		if (!ret.has_value())
		{
			return;
		}

		DataType rightType = ret.value();

		if (rightType != var->GetDataType())
		{
			CastNode* cast = new CastNode();

			cast->SetCastType(var->GetDataType());
			cast->SetExprNode(assign->RightNode());

			assign->SetRightNode(cast);
		}
	}
}

void SemaChecker::VisitReturn(ast::ReturnStmtNode* stmt)
{
	// Если функция ничего не возвращает
	if (currentFunc_->GetReturnType() == DataType::Void)
	{
		if (stmt->GetExprNode() != nullptr)
		{
			Report::PushError("Return don`t have returned expression, if function return type is void", stmt->GetSourceLocation());

			return;
		}
	}

	if (stmt->GetExprNode() == nullptr)
	{
		Report::PushError("Return must have returned value with \'" + DataTypeToString(currentFunc_->GetReturnType()) + "\' type", stmt->GetSourceLocation());

		return;
	}

	auto ret = VisitExpr(stmt->GetExprNode());

	if (ret.has_value())
	{
		if (ret.value() != currentFunc_->GetReturnType())
		{
			CastNode* cast = new CastNode();

			cast->SetCastType(currentFunc_->GetReturnType());
			cast->SetExprNode(stmt->GetExprNode());
			
			stmt->SetExprNode(cast);
		}
	}
}

void SemaChecker::VisitSeq(ast::SeqNode* seq)
{
	Traseval(seq->ExprNode());

	if (seq->NextNode() != nullptr)
	{
		VisitSeq(seq->NextNode());
	}
}

void SemaChecker::VisitFunctionDecl(ast::FunctionDeclNode* func)
{
	varDecls_.clear();

	currentFunc_ = nullptr;

	for (auto arg : func->GetArgs())
	{
		varDecls_.insert({ arg->GetName(), arg->GetVariableNode() });
	}

	currentTable_ = &func->GetSymbolTable();

	currentFunc_ = func;

	if (func->GetStartSeqNode() != nullptr)
	{
		VisitSeq(func->GetStartSeqNode());
	}
}

void SemaChecker::Traseval(ast::Node* node)
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
	}
	break;
	case ast::NodeType::ReturnStmt:
	{
		ReturnStmtNode* ret = node->AsNode<ReturnStmtNode>();

		VisitReturn(ret);
	}
		break;
	case ast::NodeType::IfStmt:
	{
		IfStmtNode* stmt = node->AsNode<IfStmtNode>();

		VisitIf(stmt);
	}
		break;
	case ast::NodeType::ElseStmt:
	{
		ElseStmtNode* stmt = node->AsNode<ElseStmtNode>();

		VisitElse(stmt);
	}
		break;
	case ast::NodeType::Stmt:
	{
		ast::StmtNode* stmt = node->AsNode<ast::StmtNode>();

		VisitStmt(stmt);
	}
	break;
	case ast::NodeType::FunctionCall:
	{
		ast::FunctionCallNode* call = node->AsNode<ast::FunctionCallNode>();

		VisitFunctionCall(call);
	}
	break;
	case NodeType::Assign:
	{
		AssignNode* assign = node->AsNode<AssignNode>();

		VisitAssign(assign);
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
		Traseval(func);
	}

	if (!status_)
	{
		std::cout << "Semantic Errors:\n";

		PrintSemaErrors();
	}
}
