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
		std::vector<Node*> parents_;

		SourceLocation loc_;

	public:
		void AddParent(Node* parent) { parents_.push_back(parent); }
		std::vector<Node*>& Parents() { return parents_; }

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
		Node* operand_ = nullptr;
		UnaryOperatorType opType_;

		DataType semaType_ = DataType::Max;

	public:
		UnaryOperatorNode(UnaryOperatorType type) : opType_(type) {}

		void SetOperand(Node* operand) 
		{ 
			if (operand != nullptr)
			{
				operand->AddParent(this);
			}

			operand_ = operand; 
		}

		Node* Operand() { return operand_; }

		UnaryOperatorType OperatorType() { return opType_; }

		void SetDataType(DataType type) { semaType_ = type; }
		DataType GetDataType() { return semaType_; }

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
		Node* leftOperand_ = nullptr;
		Node* rightOperand_ = nullptr;

		BinaryOperatorType opType_;

		/// тип выражения, выставляется на семантической фазе
		DataType semaType_ = DataType::Max;

	public:
		BinaryOperatorNode(BinaryOperatorType type) : opType_(type) {}

		void SetLeftOperand(Node* operand) 
		{ 
			leftOperand_ = operand; 

			operand->AddParent(this);
		}

		Node* LeftOperand() { return leftOperand_; }

		void SetRightOperand(Node* operand) 
		{ 
			rightOperand_ = operand;

			operand->AddParent(this);
		}

		Node* RightOperand() { return rightOperand_; }

		BinaryOperatorType OperatorType() { return opType_; }

		void SetSemaDataType(DataType type) { semaType_ = type; }
		DataType GetSemaDataType() { return semaType_; }

		NodeType Type() override
		{
			return NodeType::BinaryOperator;
		}
	};

	class ArgSeqNode : public Node
	{
	private:
		ArgSeqNode* next_ = nullptr;

		Node* expr_;

	public:
		void SetNextArgNode(ArgSeqNode* node) { next_ = node; }
		ArgSeqNode* NextArg() { return next_; }

		void SetExprNode(Node* node) { expr_ = node; }
		Node* ExprNode() { return expr_; }

		NodeType Type() override
		{
			return NodeType::ArgsSeq;
		}
	};

	class VarNode : public Node
	{
	private:
		std::string name_;
		DataType type_ = DataType::Max;

		bool isRef_ = false;
		VarNode* ref_ = nullptr;

	public:
		VarNode() = default;
		VarNode(std::string name) : name_(name) {}

		void SetName(std::string name) { name_ = name; }
		std::string& Name() { return name_; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		void SetRef(VarNode* node) { ref_ = node; }
		VarNode* GetRef() { return ref_; }

		void SetIsRef(bool state) { isRef_ = state; }
		bool IsRef() { return isRef_; }

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
		Node* assign_;

	public:
		void SetNode(Node* node) 
		{ 
			if (node != nullptr)
			{
				node->AddParent(this);
			}

			assign_ = node; 
		}

		Node* GetNode() { return assign_; }

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
		Node* expr_ = nullptr;
		SeqNode* next_ = nullptr;

	public:
		SeqNode() = default;
		SeqNode(Node* expr) : expr_(expr) {}

		void SetExprNode(Node* expr) 
		{ 
			expr_ = expr; 

			expr->AddParent(this);
		}
		
		Node* ExprNode() { return expr_; }

		void SetNextSeq(SeqNode* node) 
		{ 
			next_ = node; 

			node->AddParent(this);
		}

		SeqNode* NextNode() { return next_; }

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
		DataType type_;
		std::string name_;

		FunctionArgNode* next_ = nullptr;

		VarNode* varNode_ = nullptr;

	public:
		void SetDataType(DataType type) { this->type_ = type; }
		DataType GetDataType() { return type_; }

		void SetName(std::string name) { this->name_ = name; }
		std::string GetName() { return name_; }

		void SetNextNode(FunctionArgNode* node) { next_ = node; }
		FunctionArgNode* GetNextNode() { return next_; }

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
		DataType retValueType_;
		std::vector<FunctionArgNode*> args_;

		SeqNode* startSeq_ = nullptr;

		std::string name_;

		SymbolTable funcTable_;

		int flags_ = 0;

	public:
		void SetName(std::string name) { name_ = name; }
		std::string& GetName() { return name_; }

		void SetReturnValueType(DataType type) { retValueType_ = type; }
		DataType GetReturnType() { return retValueType_; }

		void SetFlags(int flags) { flags_ = flags; }
		int& GetFlags() { return flags_; }

		void SetStartSeqNode(SeqNode* node) 
		{ 
			startSeq_ = node; 

			node->AddParent(this);
		}
		
		SeqNode* GetStartSeqNode() { return startSeq_; }

		void AddArg(FunctionArgNode* node) { args_.push_back(node); }
		std::vector<FunctionArgNode*>& GetArgs() { return args_; }

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

		ArgSeqNode* rootArg_;
		std::string name_;

		FunctionDeclNode* callFunc_ = nullptr;

	public:
		FunctionCallNode() : rootArg_(nullptr), name_() {}
		FunctionCallNode(ArgSeqNode* rootArg, std::string name) : rootArg_(rootArg), name_(name) {}

		void SetRootArgNode(ArgSeqNode* node) { rootArg_ = node; }
		ArgSeqNode* RootArgNode() { return rootArg_; }

		void AddArg(Node* node) { args_.push_back(node); }
		std::vector<Node*>& GetArgs() { return args_; }

		void SetName(std::string name) { name_ = name; }
		std::string& Name() { return name_; }

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
		std::vector<FunctionDeclNode*> functionsDecls_;

		// Global score
		SymbolTable globalTable_;

	public:
		void AddFunctionDecl(FunctionDeclNode* node) { functionsDecls_.push_back(node); }

		std::vector<FunctionDeclNode*>& GetFunctionsDecls() { return functionsDecls_; }

		SymbolTable& GetGlobalTable() { return globalTable_; }
	};

	struct ParseError
	{
		int line;
		int column;
		std::string message;
	};
}