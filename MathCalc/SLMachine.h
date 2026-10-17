#pragma once

#include "SLM.h"

#include <vector>

namespace slm
{
	struct ConstantTable
	{
		std::vector<int> int32Vals;
		std::vector<uint32> uint32Vals;
		std::vector<float> float32Vals;
	};

	struct VMValue
	{
		void* data;
	};

	class VMachine
	{
	private:
		ubyte* stack_ = nullptr;

		ubyte* image_ = nullptr;

		uint64 imageSize_ = 0;

		uint32 codeSize_ = 0;

		ubyte* codePtr_ = nullptr;

		// Program counter
		// одна инструкция - INSTRUCTION_SIZE байта, коунтер итерируется по инструкциям, то есть чтоб узнать, где находится pc в image - m_pc * INSTRUCTION_SIZE
		uint32 pc_ = 0;

		// stack pointer
		uint32 sp_ = 0;

		int callDepth_ = 0;

		std::vector<uint32> constantTable_;

		std::vector<FunctionInfo> functions_;

	private:
		uint32 FetchInstruction();

		uint32 ReadStackUInt32(uint32 stackOffset);

		void WriteStackUInt32(uint32 value, uint32 stackOffset);

		uint32 PopUInt32();

		void ReadImageHeader();

	public:
		VMachine();

		void LoadImage(void* data, uint64 size);

		VMStatus Run();

		void WriteStackDump();
	};
}