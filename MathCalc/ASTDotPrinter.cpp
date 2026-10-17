#include "ASTDotPrinter.hpp"

#include <fstream>
#include <iostream>

#include "TypeUtils.h"

namespace ast
{
	int DotPrinter::PrintVar(VarNode* node, std::string& buffer, std::string& relationsBuff, int& id)
	{
		VarNode* var = node->AsNode<VarNode>();

		if (varNodes_.contains(var))
		{
			return varNodes_[var];
		}

		std::string prefix = "";

		if (var->IsRef())
		{
			prefix = var->Name() + " : reference";
		}
		else
		{
			prefix = var->Name();
		}

		prefix += " : " + DataTypeToString(var->GetDataType());

		buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

		buffer += "];\n";

		varNodes_.insert({ var, id });

		int ourId = id;

		id++;

		return ourId;
	}

	int DotPrinter::PrintExpr(Node* node, std::string& buffer, std::string& relationsBuff, int& id)
	{
		std::string prefix = "";

		switch (node->Type())
		{
		case NodeType::Var:
		{
			VarNode* var = node->AsNode<VarNode>();

			//if (varNodes_.contains(var))
			//{
			//	return varNodes_[var];
			//}

			//if (var->IsRef())
			//{
			//	prefix = var->Name() + " : reference";
			//}
			//else
			//{
			//	prefix = var->Name();
			//}

			//prefix += " : " + DataTypeToString(var->GetDataType());

			//buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			//buffer += "];\n";

			//varNodes_.insert({ var, id });

			//int ourId = id;

			//id++;

			return PrintVar(var, buffer, relationsBuff, id);
		}
		break;
		case NodeType::Value:
		{
			prefix = std::to_string(node->AsNode<ValueNode>()->Value());

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;

			id++;

			return ourId;
		}
		break;
		case NodeType::Cast:
		{
			CastNode* cast = node->AsNode<CastNode>();

			std::string typeTo = DataTypeToString(cast->GetCastType());

			prefix = "Cast(" + typeTo + ")";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			int exprId = PrintExpr(cast->GetExprNode(), buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(exprId) + "\n";

			return ourId;
		}
		break;
		case NodeType::BinaryOperator:
		{
			BinaryOperatorNode* opNode = node->AsNode<BinaryOperatorNode>();

			switch (opNode->OperatorType())
			{
			case BinaryOperatorType::Plus:
				prefix = "+";
				break;
			case BinaryOperatorType::Minus:
				prefix = "-";
				break;
			case BinaryOperatorType::Multiply:
				prefix = "*";
				break;
			case BinaryOperatorType::Divide:
				prefix = "/";
				break;
			case BinaryOperatorType::Less:
				prefix = "<";
				break;
			case BinaryOperatorType::Greater:
				prefix = ">";
				break;
			case BinaryOperatorType::LessEqual:
				prefix = "<=";
				break;
			case BinaryOperatorType::GreaterEqual:
				prefix = ">=";
				break;
			case BinaryOperatorType::Equal:
				prefix = "==";
				break;
			}

			prefix += " : " + DataTypeToString(opNode->GetDataType());

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			int leftId = PrintExpr(opNode->LeftOperand(), buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(leftId) + "\n";

			//PrintNode(opNode->LeftOperand(), buffer, relationsBuff, id);

			//id++;

			int rightId = PrintExpr(opNode->RightOperand(), buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(rightId) + "\n";

			//PrintNode(opNode->RightOperand(), buffer, relationsBuff, id);

			return ourId;
		}
		break;
		case NodeType::FunctionCall:
		{
			FunctionCallNode* func = node->AsNode<FunctionCallNode>();

			prefix = "Function Call \'" + func->Name() + "\'";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

			PrintNode(func->RootArgNode(), buffer, relationsBuff, id);

			return ourId;
		}
		break;
		case NodeType::UnaryOperator:
		{
			UnaryOperatorNode* opNode = node->AsNode<UnaryOperatorNode>();

			switch (opNode->OperatorType())
			{
			case UnaryOperatorType::UnaryMinus:
				prefix = "-value";
				break;
			}

			prefix += " : " + DataTypeToString(opNode->GetDataType());

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			int opId = PrintExpr(opNode->Operand(), buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(opId) + "\n";

			//PrintNode(opNode->Operand(), buffer, relationsBuff, id);

			return ourId;
		}
		}

		return -1;
	}

	int DotPrinter::PrintElseStmt(ElseStmtNode* stmt, std::string& buffer, std::string& relationsBuff, int& id)
	{
		if (elseStmts_.contains(stmt))
		{
			return elseStmts_[stmt];
		}

		std::string prefix = "";

		prefix = "Else";
		buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

		buffer += "];\n";

		int ourId = id;
		id++;

		if (stmt->GetStartSeq() != nullptr)
		{
			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "[label=\"Else body\"]" + "\n";

			PrintNode(stmt->GetStartSeq(), buffer, relationsBuff, id);
		}

		elseStmts_.insert({ stmt, ourId });

		return ourId;
	}

	void DotPrinter::PrintNode(Node* node, std::string& buffer, std::string& relationsBuff, int& id)
	{
		std::string prefix = "";

		switch (node->Type())
		{
		case NodeType::Seq:
		{
			SeqNode* seq = node->AsNode<SeqNode>();

			prefix = "Seq";

			int ourId = id;
			id++;

			if (seq->ExprNode() != nullptr)
			{
				buffer += "a" + std::to_string(ourId) + " [label=\"" + prefix + "\" ";

				buffer += "];\n";

				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "[label=\"Seq content\"]" + "\n";

				PrintNode(seq->ExprNode(), buffer, relationsBuff, id);
			}
			else
			{
				buffer += "a" + std::to_string(ourId) + " [label=\"" + prefix + " | Empty" + "\" ";

				buffer += "];\n";
			}

			if (seq->NextNode() != nullptr)
			{
				id++;

				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

				PrintNode(seq->NextNode(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		case NodeType::Stmt:
		{
			StmtNode* stmt = node->AsNode<StmtNode>();

			prefix = "Stmt";

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

			if (stmt->GetNode() != nullptr)
			{
				PrintNode(stmt->GetNode(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		case NodeType::FunctionArg:
		{
			FunctionArgNode* arg = node->AsNode<FunctionArgNode>();

			prefix = "Function Arg '" + arg->GetName() + "'";

			prefix += " : " + DataTypeToString(arg->GetDataType());

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			return;
		}
		break;
		case NodeType::FunctionDecl:
		{
			FunctionDeclNode* decl = node->AsNode<FunctionDeclNode>();

			prefix = "Function Decl '" + decl->GetName() + "'";

			prefix += " : " + DataTypeToString(decl->GetReturnType());

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			for (auto arg : decl->GetArgs())
			{
				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

				PrintNode(arg, buffer, relationsBuff, id);

				id++;
			}

			if (decl->GetStartSeqNode() != nullptr)
			{
				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "[label=\"Function body\"]" + "\n";

				PrintNode(decl->GetStartSeqNode(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		case NodeType::Value:
		{
			prefix = std::to_string(node->AsNode<ValueNode>()->Value());
		}
		break;
		case NodeType::UnaryOperator:
		{
			PrintExpr(node, buffer, relationsBuff, id);
			/*UnaryOperatorNode* opNode = node->AsNode<UnaryOperatorNode>();

			switch (opNode->OperatorType())
			{
			case UnaryOperatorType::UnaryMinus:
				prefix = "-value";
				break;
			}

			prefix += " : " + DataTypeToString(opNode->GetDataType());

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

			PrintNode(opNode->Operand(), buffer, relationsBuff, id);*/

			return;
		}
		case NodeType::BinaryOperator:
		{
			PrintExpr(node, buffer, relationsBuff, id);

			//BinaryOperatorNode* opNode = node->AsNode<BinaryOperatorNode>();

			//switch (opNode->OperatorType())
			//{
			//case BinaryOperatorType::Plus:
			//	prefix = "+";
			//	break;
			//case BinaryOperatorType::Minus:
			//	prefix = "-";
			//	break;
			//case BinaryOperatorType::Multiply:
			//	prefix = "*";
			//	break;
			//case BinaryOperatorType::Divide:
			//	prefix = "/";
			//	break;
			//case BinaryOperatorType::Less:
			//	prefix = "<";
			//	break;
			//case BinaryOperatorType::Greater:
			//	prefix = ">";
			//	break;
			//case BinaryOperatorType::LessEqual:
			//	prefix = "<=";
			//	break;
			//case BinaryOperatorType::GreaterEqual:
			//	prefix = ">=";
			//	break;
			//case BinaryOperatorType::Equal:
			//	prefix = "==";
			//	break;
			//}

			//prefix += " : " + DataTypeToString(opNode->GetDataType());

			//buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			//buffer += "];\n";

			//int ourId = id;
			//id++;

			//int leftId = PrintOperator(opNode->LeftOperand(), buffer, relationsBuff, id);

			//relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(leftId) + "\n";

			////PrintNode(opNode->LeftOperand(), buffer, relationsBuff, id);

			////id++;

			//int rightId = PrintOperator(opNode->RightOperand(), buffer, relationsBuff, id);

			//relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(rightId) + "\n";

			////PrintNode(opNode->RightOperand(), buffer, relationsBuff, id);

			return;
		}
		break;
		/*case NodeType::FunctionCall:
		{
			FunctionCallNode* func = node->AsNode<FunctionCallNode>();

			prefix = "Function Call \'" + func->Name() + "\'";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

			PrintNode(func->RootArgNode(), buffer, relationsBuff, id);

			return;
		}
		break;*/
		case NodeType::Assign:
		{
			AssignNode* assign = node->AsNode<AssignNode>();

			prefix = "Assign (=)";

			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			VarNode* var = assign->LeftNode()->AsNode<VarNode>();

			int varId = PrintVar(var, buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(varId) + "\n";

			//PrintNode(assign->LeftNode(), buffer, relationsBuff, id);

			id++;

			int exprId = PrintExpr(assign->RightNode(), buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(exprId) + "\n";

			//PrintNode(assign->RightNode(), buffer, relationsBuff, id);

			return;
		}
		break;
		//case NodeType::Var:
		//{
		//	VarNode* var = node->AsNode<VarNode>();

		//	//if (varNodes_.contains(var))
		//	//{
		//	//	return;
		//	//}

		//	//if (var->IsRef())
		//	//{
		//	//	prefix = var->Name() + " : reference";
		//	//}
		//	//else
		//	//{
		//	//	prefix = var->Name();
		//	//}

		//	//prefix += " : " + DataTypeToString(var->GetDataType());

		//	//buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

		//	//buffer += "];\n";

		//	//varNodes_.insert({ var, id });

		//	//id++;

		//	PrintVar(var, buffer, relationsBuff, id);

		//	return;
		//}
		//break;
		//case NodeType::Cast:
		//{
		//	CastNode* cast = node->AsNode<CastNode>();

		//	std::string typeTo = DataTypeToString(cast->GetCastType());

		//	prefix = "Cast(" + typeTo + ")";
		//	buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

		//	buffer += "];\n";

		//	int ourId = id;
		//	id++;

		//	int exprId = PrintExpr(cast->GetExprNode(), buffer, relationsBuff, id);

		//	relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(exprId) + "\n";

		//	//PrintNode(cast->GetExprNode(), buffer, relationsBuff, id);

		//	return;
		//}
		//break;
		case NodeType::ArgsSeq:
		{
			ArgSeqNode* args = node->AsNode<ArgSeqNode>();

			prefix = "Arg";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			int exprNode = PrintExpr(args->ExprNode(), buffer, relationsBuff, id);

			relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(exprNode) + "\n";

			//PrintNode(args->ExprNode(), buffer, relationsBuff, id);

			if (args->NextArg() != nullptr)
			{
				id++;

				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "\n";

				PrintNode(args->NextArg(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		case NodeType::ReturnStmt:
		{
			ReturnStmtNode* ret = node->AsNode<ReturnStmtNode>();

			prefix = "Return";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			if (ret->GetExprNode() != nullptr)
			{
				int exprId = PrintExpr(ret->GetExprNode(), buffer, relationsBuff, id);
				//id++;

				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(exprId) + "\n";

				//PrintNode(ret->GetExprNode(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		case NodeType::IfStmt:
		{
			IfStmtNode* stmt = node->AsNode<IfStmtNode>();

			prefix = "If";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			if (stmt->GetCond() != nullptr)
			{
				int condId = PrintExpr(stmt->GetCond(), buffer, relationsBuff, id);

				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(condId) + "[label=\"If Cond\"]" + "\n";

				//PrintNode(stmt->GetCond(), buffer, relationsBuff, id);
			}

			if (stmt->GetStartSeq() != nullptr)
			{
				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "[label=\"If body\"]" + "\n";

				PrintNode(stmt->GetStartSeq(), buffer, relationsBuff, id);
			}


			if (stmt->GetElseStmt() != nullptr)
			{
				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "[label=\"Else block\"]" + "\n";

				PrintNode(stmt->GetElseStmt(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		case NodeType::ElseStmt:
		{
			ElseStmtNode* stmt = node->AsNode<ElseStmtNode>();

			PrintElseStmt(stmt, buffer, relationsBuff, id);

			prefix = "Else";
			buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

			buffer += "];\n";

			int ourId = id;
			id++;

			if (stmt->GetStartSeq() != nullptr)
			{
				relationsBuff += "a" + std::to_string(ourId) + "->a" + std::to_string(id) + "[label=\"Else body\"]" + "\n";

				PrintNode(stmt->GetStartSeq(), buffer, relationsBuff, id);
			}

			return;
		}
		break;
		default:
			throw std::exception("Not implemeted");
		}

		buffer += "a" + std::to_string(id) + " [label=\"" + prefix + "\" ";

		buffer += "];\n";

		id++;
	}

	void DotPrinter::Print(ast::Context& context, std::string graphName, bool isCyclic)
	{
		std::string buffer;
		std::string relationsBuff;

		buffer += "digraph ast {\n";

		buffer += "node [shape=box];\n";

		if (isCyclic)
		{
			//buffer += "layout = circo;\n";
		}

		int id = 0;

		for (auto func : context.GetFunctionsDecls())
		{
			varNodes_.clear();
			elseStmts_.clear();

			PrintNode(func, buffer, relationsBuff, id);

			id++;
		}

		buffer += "\n";

		buffer += relationsBuff;

		buffer += "}";

		std::string fileName = graphName + ".dot";

		std::ofstream file(fileName, std::ios::out);

		file.write(buffer.c_str(), buffer.size());

		file.close();
	}
}