#pragma once

#include "Token.hpp"
#include "DataType.hpp"

#include "SymbolTable.hpp"

#include <vector>
#include <map>

namespace ast
{
	enum class NodeType
	{
		Value,
		UnaryOperator,
		BinaryOperator,
		ArgsSeq,
		FunctionCall,
		Var,
		Assign,
		ReturnStmt,
		Stmt,
		IfStmt,
		ElseStmt,
		Seq,
		Cast,
		FunctionDecl,
		FunctionArg
	};

	class Node
	{
	private:
		std::vector<Node*> parents;

		SourceLocation loc_;

	public:
		void AddParent(Node* parent) { parents.push_back(parent); }
		std::vector<Node*>& Parents() { return parents; }

		void SetSourceLocation(SourceLocation loc) { loc_ = loc; }
		SourceLocation& GetSourceLocation() { return loc_; }

		template<typename TypeT>
		TypeT* AsNode()
		{
			return dynamic_cast<TypeT*>(this);
		}

		virtual NodeType Type() = 0;
	};


	class ValueNode : public Node
	{
	private:
		int value_ = int();
		DataType type_ = DataType::Max;

	public:
		ValueNode(int value) : value_(value) {}

		void SetValue(int value) { value_ = value; }
		int Value() { return value_; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		NodeType Type() override
		{
			return NodeType::Value;
		}
	};

	enum class UnaryOperatorType
	{
		UnaryMinus,
	};

	class UnaryOperatorNode : public Node
	{
	private:
		Node* m_operand = nullptr;
		UnaryOperatorType m_type;

		DataType type_ = DataType::Max;

	public:
		UnaryOperatorNode(UnaryOperatorType type) : m_type(type) {}

		void SetOperand(Node* operand) 
		{ 
			if (operand != nullptr)
			{
				operand->AddParent(this);
			}

			m_operand = operand; 
		}

		Node* Operand() { return m_operand; }

		UnaryOperatorType OperatorType() { return m_type; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		NodeType Type() override
		{
			return NodeType::UnaryOperator;
		}
	};

	enum class BinaryOperatorType
	{
		Plus,
		Minus,
		Multiply,
		Divide,
		Greater, // >
		Less, // <
		Equal, // ==
		GreaterEqual, // >=
		LessEqual, // <=
	};

	class BinaryOperatorNode : public Node
	{
	private:
		Node* m_leftOperand = nullptr;
		Node* m_rightOperand = nullptr;

		BinaryOperatorType m_type;

		/// тип выражения, выставляется на семантической фазе
		DataType type_ = DataType::Max;

	public:
		BinaryOperatorNode(BinaryOperatorType type) : m_type(type) {}

		void SetLeftOperand(Node* operand) 
		{ 
			m_leftOperand = operand; 

			operand->AddParent(this);
		}

		Node* LeftOperand() { return m_leftOperand; }

		void SetRightOperand(Node* operand) 
		{ 
			m_rightOperand = operand;

			operand->AddParent(this);
		}

		Node* RightOperand() { return m_rightOperand; }

		BinaryOperatorType OperatorType() { return m_type; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		NodeType Type() override
		{
			return NodeType::BinaryOperator;
		}
	};

	class ArgSeqNode : public Node
	{
	private:
		ArgSeqNode* m_next = nullptr;

		Node* m_expr;
	public:
		void SetNextArgNode(ArgSeqNode* node) { m_next = node; }
		ArgSeqNode* NextArg() { return m_next; }

		void SetExprNode(Node* node) { m_expr = node; }
		Node* ExprNode() { return m_expr; }

		NodeType Type() override
		{
			return NodeType::ArgsSeq;
		}
	};

	class VarNode : public Node
	{
	private:
		std::string m_name;
		DataType type = DataType::Max;

		bool m_isRef = false;
		VarNode* m_ref = nullptr;

	public:
		VarNode() = default;
		VarNode(std::string name) : m_name(name) {}

		void SetName(std::string name) { m_name = name; }
		std::string& Name() { return m_name; }

		void SetDataType(DataType type) { this->type = type; }
		DataType GetDataType() { return type; }

		void SetRef(VarNode* node) { m_ref = node; }
		VarNode* GetRef() { return m_ref; }

		void SetIsRef(bool state) { m_isRef = state; }
		bool IsRef() { return m_isRef; }

		NodeType Type() override
		{
			return NodeType::Var;
		}
	};

	class AssignNode : public Node
	{
	private:
		Node* m_left = nullptr;
		Node* m_right = nullptr;

	public:
		AssignNode(Node* left, Node* right) : m_left(left), m_right(right) {}

		void SetLeftNode(Node* node) { m_left = node; }
		Node* LeftNode() { return m_left; }

		void SetRightNode(Node* node) { m_right = node; }
		Node* RightNode() { return m_right; }

		NodeType Type() override
		{
			return NodeType::Assign;
		}
	};

	class StmtNode : public Node
	{
	private:
		Node* m_assign;

	public:
		void SetNode(Node* node) 
		{ 
			if (node != nullptr)
			{
				node->AddParent(this);
			}

			m_assign = node; 
		}

		Node* GetNode() { return m_assign; }

		NodeType Type() override
		{
			return NodeType::Stmt;
		}
	};

	class ReturnStmtNode : public Node
	{
	private:
		Node* exprNode_ = nullptr;

	public:
		void SetExprNode(Node* node) { exprNode_ = node; }
		Node* GetExprNode() { return exprNode_; }

		NodeType Type() override
		{
			return NodeType::ReturnStmt;
		}
	};

	class CastNode : public Node
	{
	private: 
		Node* exprNode_ = nullptr;

		DataType typeCastTo_;

	public:
		void SetExprNode(Node* node) { exprNode_ = node; }
		Node* GetExprNode() { return exprNode_; }

		void SetCastType(DataType type) { typeCastTo_ = type; }
		DataType GetCastType() { return typeCastTo_; }

		NodeType Type() override
		{
			return NodeType::Cast;
		}

	};

	class SeqNode : public Node
	{
	private:
		Node* m_expr = nullptr;
		SeqNode* m_next = nullptr;

	public:
		SeqNode() = default;
		SeqNode(Node* expr) : m_expr(expr) {}

		void SetExprNode(Node* expr) 
		{ 
			m_expr = expr; 

			expr->AddParent(this);
		}
		
		Node* ExprNode() { return m_expr; }

		void SetNextSeq(SeqNode* node) 
		{ 
			m_next = node; 

			node->AddParent(this);
		}

		SeqNode* NextNode() { return m_next; }

		NodeType Type() override
		{
			return NodeType::Seq;
		}
	};

	class ElseStmtNode;

	class IfStmtNode : public Node
	{
	private:
		Node* cond_ = nullptr;

		SeqNode* startSeq_ = nullptr;

		ElseStmtNode* else_ = nullptr;

	public:
		void SetCond(Node* cond)
		{
			if (cond != nullptr)
			{
				cond->AddParent(this);
			}

			cond_ = cond;
		}

		Node* GetCond() { return cond_; }

		void SetStartSeq(SeqNode* seq)
		{
			if (seq != nullptr)
			{
				seq->AddParent(this);
			}

			startSeq_ = seq;
		}

		SeqNode* GetStartSeq() { return startSeq_; }

		void SetElseStmt(ElseStmtNode* node)
		{
			else_ = node;
		}

		ElseStmtNode* GetElseStmt() { return else_; }

		NodeType Type() override
		{
			return NodeType::IfStmt;
		}
	};

	class ElseStmtNode : public Node
	{
	private:
		SeqNode* startSeq_ = nullptr;

		IfStmtNode* if_ = nullptr;

	public:
		void SetStartSeq(SeqNode* seq)
		{
			if (seq != nullptr)
			{
				seq->AddParent(this);
			}

			startSeq_ = seq;
		}

		SeqNode* GetStartSeq() { return startSeq_; }

		void SetIfStmt(IfStmtNode* node)
		{
			if_ = node;

			if (node != nullptr)
			{
				if_->SetElseStmt(this);
			}
		}

		IfStmtNode* GetIfStmt() { return if_; }

		NodeType Type() override
		{
			return NodeType::ElseStmt;
		}
	};

	class BlockNode : public Node
	{

	};

	class FunctionArgNode : public Node
	{
	private:
		DataType type;
		std::string name;

		FunctionArgNode* next = nullptr;

		VarNode* varNode_;

	public:
		void SetDataType(DataType type) { this->type = type; }
		DataType GetDataType() { return type; }

		void SetName(std::string name) { this->name = name; }
		std::string GetName() { return name; }

		void SetNextNode(FunctionArgNode* node) { next = node; }
		FunctionArgNode* GetNextNode() { return next; }

		void SetVariableNode(VarNode* id) { varNode_ = id; }
		VarNode* GetVariableNode() { return varNode_; }

		NodeType Type() override
		{
			return NodeType::FunctionArg;
		}
	};

	enum FunctionFlags
	{
		FunctionExtern = 1 << 0
	};

	class FunctionDeclNode : public Node
	{
	private:
		DataType retValueType;
		std::vector<FunctionArgNode*> args;

		SeqNode* startSeq = nullptr;

		std::string name;

		SymbolTable funcTable_;

		int flags_ = 0;

	public:
		void SetName(std::string name) { this->name = name; }
		std::string& GetName() { return name; }

		void SetReturnValueType(DataType type) { retValueType = type; }
		DataType GetReturnType() { return retValueType; }

		void SetFlags(int flags) { flags_ = flags; }
		int& GetFlags() { return flags_; }

		void SetStartSeqNode(SeqNode* node) 
		{ 
			startSeq = node; 

			node->AddParent(this);
		}
		
		SeqNode* GetStartSeqNode() { return startSeq; }

		void AddArg(FunctionArgNode* node) { args.push_back(node); }
		std::vector<FunctionArgNode*>& GetArgs() { return args; }

		SymbolTable& GetSymbolTable() { return funcTable_; }

		NodeType Type() override
		{
			return NodeType::FunctionDecl;
		}
	};

	class FunctionCallNode : public Node
	{
	private:
		std::vector<Node*> args_;

		ArgSeqNode* m_rootArg;
		std::string m_name;

		FunctionDeclNode* callFunc_ = nullptr;

	public:
		FunctionCallNode() : m_rootArg(nullptr), m_name() {}
		FunctionCallNode(ArgSeqNode* rootArg, std::string name) : m_rootArg(rootArg), m_name(name) {}

		void SetRootArgNode(ArgSeqNode* node) { m_rootArg = node; }
		ArgSeqNode* RootArgNode() { return m_rootArg; }

		void AddArg(Node* node) { args_.push_back(node); }
		std::vector<Node*>& GetArgs() { return args_; }

		void SetName(std::string name) { m_name = name; }
		std::string& Name() { return m_name; }

		void SetCallFunction(FunctionDeclNode* node) { callFunc_ = node; }
		FunctionDeclNode* GetCallFunction() { return callFunc_; }

		NodeType Type() override
		{
			return NodeType::FunctionCall;
		}
	};

	class Context
	{
	private:
		std::vector<FunctionDeclNode*> functionsDecls;

		// Global score
		SymbolTable globalTable_;

	public:
		void AddFunctionDecl(FunctionDeclNode* node) { functionsDecls.push_back(node); }

		std::vector<FunctionDeclNode*>& GetFunctionsDecls() { return functionsDecls; }

		SymbolTable& GetGlobalTable() { return globalTable_; }
	};

	struct ParseError
	{
		int line;
		int column;
		std::string message;
	};
}