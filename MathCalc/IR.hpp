#pragma 

#include <string>
#include <vector>

#include "DataType.hpp"

namespace ir
{
	struct IntConstant
	{
		int value;
	};

	enum class ElementType
	{
		Instruction,
		Constant,
		FunctionArg
	};

	/**
	* @brief элемент IR - инструкция, константа, etc
	*/
	class Element
	{
	private:
		std::vector<Element*> users_;

	public:
		void AddUser(Element* elem) { users_.push_back(elem); }
		std::vector<Element*>& GetUsers() { return users_; }

		virtual ElementType GetType() = 0;
	};

	enum class InstructionType
	{
		BinaryOp,
		Load,
		Store,
		FunctionCall,
		Cast
	};

	enum class BinaryOpType
	{
		eAdd,
		eSub,
		Div,
		Mul,
	};

	class Instruction : public Element
	{
	private:
		/// используется для названия возвращаемого результата (например, в текстовом представлении)
		std::string debugName_;

	public:
		void SetDebugName(std::string& name) { debugName_ = name; }
		std::string GetDebugName() { return debugName_; }

		ElementType GetType() override
		{
			return ElementType::Instruction;
		}

		virtual InstructionType GetInstructionType() = 0;
	};

	class BinaryOp : public Instruction
	{
	private:
		Element* first_ = nullptr;
		Element* second_ = nullptr;

		BinaryOpType opType_;

		/// Тип, с которыми оперирует инструкция.
		DataType type_;

	public:
		void SetFisrt(Element* elem) { first_ = elem; }
		Element* GetFirst() { return first_; }

		void SetSecond(Element* elem) { second_ = elem; }
		Element* GetSecond() { return second_; }

		void SetBinaryOpType(BinaryOpType type) { opType_ = type; }
		BinaryOpType GetBinaryOpType() { return opType_; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		InstructionType GetInstructionType() override
		{
			return InstructionType::BinaryOp;
		}
	};

	class LoadOp : public Instruction
	{
	private:
		Element* src_ = nullptr;

		DataType type_;

	public:
		void SetSource(Element* elem) { src_ = elem; }
		Element* GetSource() { return src_; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		InstructionType GetInstructionType() override
		{
			return InstructionType::Load;
		}
	};

	class StoreOp : public Instruction
	{
	private:
		Element* dst_ = nullptr;

		DataType type_;

	public:
		void SetDestination(Element* elem) { dst_ = elem; }
		Element* GetDestination() { return dst_; }

		void SetDataType(DataType type) { type_ = type; }
		DataType GetDataType() { return type_; }

		InstructionType GetInstructionType() override
		{
			return InstructionType::Store;
		}
	};

	class Constant : public Element
	{
	private:
		DataType type_ = DataType::Max;

		void* value_ = nullptr;

	public:
		Constant(DataType type)
		{
			if (type == DataType::Void)
			{
				// ассерт, воид не может быть константой
				return;
			}

			if (type == DataType::Max)
			{
				// ассерт, состояние неизвестно
				return;
			}

			type_ = type;
		}

		void SetInt32(int value) 
		{
			if (type_ != DataType::Int32)
			{
				return;
			}

			*((int*)value_) = value;
		}

		int GetInt32()
		{
			if (type_ != DataType::Int32)
			{
				//TODO тут должен быть ассерт
				return -1;
			}

			if (value_ == nullptr)
			{
				// TODO тут должен быть ассерт
				return -1;
			}

			return *((int*)value_);
		}

		void SetPointer(void* ptr) { value_ = ptr; }
		void* GetPointer() { return value_; }

		ElementType GetType() override
		{
			return ElementType::Constant;
		}
	};

	class FunctionArg : public Element
	{
	private:
		Element* value_ = nullptr;

		std::string name_;

	public:
		void SetName(std::string& name) { name_ = name; }
		std::string GetName() { return name_; }

		void SetValue(Element* elem) { value_ = elem; }
		Element* GetValue() { return value_; }

		ElementType GetType() override
		{
			return ElementType::FunctionArg;
		}
	};

	class Function;

	class FunctionCall : public Instruction
	{
	private:
		Function* callFunc_ = nullptr;

		std::vector<Element*> args_;

	public:
		void SetFunction(Function* func) { callFunc_ = func; }
		Function* GetFunction() { return callFunc_; }

		void AddArg(Element* elem) { args_.push_back(elem); }
		std::vector<Element*>& GetArgs() { return args_; }

		InstructionType GetInstructionType() override
		{
			return InstructionType::FunctionCall;
		}
	};

	class Cast : public Instruction
	{
	private:
		Element* arg_ = nullptr;

		DataType typeFrom_;

		DataType typeTo_;

	public:
		void SetArg(Element* elem) { arg_ = elem; }
		Element* GetArg() { return arg_; }

		void SetTypeFrom(DataType type) { typeFrom_ = type; }
		DataType GetTypeFrom() { return typeFrom_; }

		void SetTypeTo(DataType type) { typeTo_ = type; }
		DataType GetTypeTo() { return typeTo_; }

		InstructionType GetInstructionType() override
		{
			return InstructionType::Cast;
		}
	};

	class Block
	{
	private:
		std::vector<Element*> elements_;

		Block* prev_ = nullptr;
		Block* next_ = nullptr;

		std::string label_;

	public:
		Block(std::string& label) : label_(label) {}

		void AddElement(Element* elem) { elements_.push_back(elem); }
		std::vector<Element*> GetElements() { return elements_; }

		void SetPrevBlock(Block* block) { prev_ = block; }
		Block* GetPrevBlock() { return prev_; }

		void SetNextBlock(Block* block) { next_ = block; }
		Block* GetNextBlock() { return next_; }

		std::string GetLabel() { return label_; }
	};

	class Function
	{
	private:
		std::vector<Block*> blocks_;

		std::vector<FunctionArg*> args_;

		std::string name_;

	public:
		Function(std::string& name) : name_(name) {}

		void AddBlock(Block* block) { blocks_.push_back(block); }
		std::vector<Block*>& GetBlocks() { return blocks_; }

		void AddArg(FunctionArg* arg) { args_.push_back(arg); }
		std::vector<FunctionArg*>& GetArgs() { return args_; }

		std::string& GetName() { return name_; }
	};

	class Program
	{
	private:
		std::vector<Function*> functions_;

	public:
		void AddFunction(Function* func) { functions_.push_back(func); }
		std::vector<Function*>& GetFunctions() { return functions_; }
	};
}