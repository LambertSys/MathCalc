#pragma once

#include <string>
#include <vector>

#include <unordered_map>

#include "SLM.h"

namespace slm
{
	enum class TokenType
	{
		Op, // mov, add, etc
		MathOp, // +, * (нужно для косвеной адресации)
		Constant,
		Comma, // ,
		Function, // function keyword
		End, // end keyword
		Colon, // :
		Ret, // ret keyword
	};

	/*
	* <command> <dest> <src>
	* 
	* mov sp, 45
	* mov [sp + 4], 42
	* add [sp + 4], sp -- по адресу sp+4 будет хранится 87
	* 
	* function Lol:
	* 
	* end
	*/
	class Lexer
	{
	public:
		bool Run(std::string file);
	};

	struct ImageInfo
	{
		ubyte* ptr;
		uint32 size;
	};

	class AsmBuilder
	{
	private:

		std::vector<uint32> buffer_;

		std::vector<uint32> constantTable_;

		std::vector<FunctionInfo> functions_;

		std::unordered_map<std::string, uint64> funcIndices_;

		std::unordered_map<uint32, uint16> constantCaches_;

		uint16 currentConstantIndex_ = 0;

		FunctionInfo currentFunc_;

		int entryFuncIdx_ = -1;

		bool isEmpty_ = true;

	private:
		/**
		 * @brief записывает константу, возвращая индекс в таблице констант
		 * @param constant 
		 * @return 
		 */
		uint16 WriteConstant(uint32 constant);

		void WriteInstruction(uint32 instr);

		void ClearState();

		void WriteMetadata(ubyte** sectionPtr, uint32& size);

	public:
		void WriteFunction(std::string& name);

		void SetEntryFunction(std::string& name);

		/**
		* Mov encoding
		* opcode - 0...7 bits - 8 bits len
		* flags - 8...9 bits - 2 bits len
		* dst - 10...19 bits - 10 bits len
		* src - 20...29 bits - 10 bits len
		* 30-31 bits - 0-filling
		* @brief
		* @param dst адрес стека, куда будет записано значение
		* @param src с флагом mov_flags::USE_CONSTANT_SRC будет константой, записанной по dst адресу. С флагом mov_flags::USE_STACK_SRC будет адресом в стеке
		* @param flags
		*/         
		void WriteMov(uint32 dst, uint32 src, uint8 flags);

		/**
		* add encoding
		* opcode - 0...7 - 8 bits len
		* dst - 8...17 - 10 bits len
		* src - 18...27 - 10 bits len
		 * @brief складывает два значения из стека по смещениям, ложа результат по смещению dst
		 * @param dst 
		 * @param src 
		 */
		void WriteAdd(uint32 dst, uint32 src);

		/**
		* Call encoding
		* opcode - 0...7 bits - 8 bits len
		* offset - 8...17 bits - 10 bits len
		* 18...31 - 0-filling
		*/
		void WriteCall(std::string& funcName);

		/**
		* Call encoding
		* opcode - 0...7 bits - 8 bits len
		* offset - 8...17 bits - 10 bits len
		* 18...31 - 0-filling
		*/
		void WriteRet();

		/**
		* Breakpoint encoding
		* opcode - 0...7 bits - 8 bits len
		* 8...31 bits - zero-filling
		*/
		void WriteBreakPoint();

		ImageInfo GetImage();
	};
}