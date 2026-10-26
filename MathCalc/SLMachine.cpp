#include "SLMachine.h"

#include <cstring>
#include <fstream>

namespace slm
{
	uint32 VMachine::FetchInstruction()
	{
		uint32* ptr = reinterpret_cast<uint32*>(codePtr_ + (pc_ * INSTRUCTION_SIZE));

		pc_++;

		return *ptr;
	}

	uint32 VMachine::ReadStackUInt32(uint32 stackOffset)
	{
		uint32* ptr = reinterpret_cast<uint32*>(stack_ + stackOffset);

		return *ptr;
	}

	void VMachine::WriteStackUInt32(uint32 value, uint32 stackOffset)
	{
		uint32* ptr = reinterpret_cast<uint32*>(stack_ + stackOffset);

		*ptr = value;
	}

	uint32 VMachine::PopUInt32()
	{
		uint32* ptr = reinterpret_cast<uint32*>(stack_ + sp_);

		sp_ -= sizeof(uint32);

		return *ptr;
	}

	void VMachine::ReadImageHeader()
	{
		SLMHeader& header = *reinterpret_cast<SLMHeader*>(image_);

		codeSize_ = header.codeSize;

		codePtr_ = image_ + sizeof(SLMHeader) + header.metadataSize;

		MetadataHeader& met = *reinterpret_cast<MetadataHeader*>(image_ + sizeof(SLMHeader));

		functions_.resize(met.functionsCount);

		uint32 offset = 0;

		for (uint32 i = 0; i < met.functionsCount; i++)
		{
			ubyte* funcPtr = image_ + sizeof(SLMHeader) + sizeof(MetadataHeader) + offset;

			uint16 nameLen = *((uint16*)funcPtr);

			std::string name((char*)(funcPtr + sizeof(uint16)), nameLen);

			uint32 offset = *((uint32*)(funcPtr + sizeof(uint16) + nameLen + 1));

			FunctionInfo info;

			info.name = std::move(name);
			info.offset = offset;

			functions_[i] = info;

			offset += sizeof(uint16) + nameLen + 1 + sizeof(uint32);
		}

		pc_ = functions_[header.entryFunctionIdx].offset;

		uint16 constCount = header.constantTableCount;

		uint32* tablePtr = reinterpret_cast<uint32*>(image_ + sizeof(SLMHeader) + codeSize_ + header.metadataSize);

		constantTable_.resize(constCount);

		for (int i = 0; i < constCount; i++)
		{
			constantTable_[i] = *(tablePtr + i);
		}
	}

	VMachine::VMachine()
	{
		// 64 kb
		stack_ = new ubyte[1 * 1024 * 64];

		// 128 kb
		image_ = new ubyte[INSTRUCTION_SIZE * 1024 * 64];
	}

	void VMachine::LoadImage(void* data, uint64 size)
	{
		memcpy(image_, data, size);

		imageSize_ = size;

		ReadImageHeader();
	}

	VMStatus VMachine::Run()
	{
		callDepth_ = 1;

		// Для entry функции должен быть ret, хоть и управление никуда не будет возвращаться. 4 байта будут заняты пустотой, 
		// чтоб ret от entry-функции не увел sp в переполнение
		sp_ = 4;

		while (callDepth_ > 0)
		{
			uint32 instr = FetchInstruction();

			uint8 opcode = static_cast<uint8>(instr & 0xff);

			switch (opcode)
			{
			case InstructionType::eAdd:
			{
				uint16 dstIndex = static_cast<uint16>((instr & 0x3ff00) >> 8);

				uint32 dstOffset = constantTable_[dstIndex];

				uint16 srcIndex = static_cast<uint16>((instr & 0xffc0000) >> 18);

				uint32 srcOffset = constantTable_[srcIndex];

				uint32 dst = ReadStackUInt32(dstOffset);
				uint32 src = ReadStackUInt32(srcOffset);

				WriteStackUInt32(dst + src, dstOffset);
			}
			break;
			case InstructionType::eSub:
			{
				//int dst = ReadStackInt32(firstArgAddr);
				//int src = ReadStackInt32(secondArgAddr);

				//WriteStackInt32(dst - src, firstArgAddr);
			}
			break;
			case InstructionType::eMov:
			{
				uint16 dstIndex = static_cast<uint16>((instr & 0xffc00) >> 10);

				uint32 dst = constantTable_[dstIndex];

				uint16 srcIndex = static_cast<uint16>((instr & 0x3ff00000) >> 20);

				uint8 mode = static_cast<uint8>((instr & 0x300) >> 8);

				if (mode == mov_flags::USE_CONSTANT_SRC)
				{
					uint32 src = constantTable_[srcIndex];

					WriteStackUInt32(src, dst);
				}
				else if (mode == mov_flags::USE_STACK_SRC)
				{
					uint32 srcOffset = constantTable_[srcIndex];
					uint32 src = ReadStackUInt32(srcOffset);

					WriteStackUInt32(src, dst);
				}
			}
			break;
			case InstructionType::eBrkPoint:
			{
				//__debugbreak();
			}
			break;
			case InstructionType::eCall:
			{

			}
			break;
			case InstructionType::eRet:
			{
				callDepth_--;

				pc_ = PopUInt32();
			}
			break;
			}
		}

		return VMStatus::Finish;
	}

	void VMachine::WriteStackDump()
	{
		std::ofstream file;

		file.open("stack.bin", std::ios::binary);

		file.write((char*)stack_, 1 * 1024 * 64);

		file.close();
	}

}

