#include "SLAsm.h"

#include <algorithm>

#include "Utils.h"
#include "Report.hpp"

namespace slm
{
	bool Lexer::Run(std::string file)
	{
		return true;
	}

	uint16 AsmBuilder::WriteConstant(uint32 constant)
	{
		if (constantCaches_.contains(constant))
		{
			return constantCaches_[constant];
		}

		uint16 currId = currentConstantIndex_;

		constantTable_.push_back(constant);
		constantCaches_.insert({ constant, currId });

		currentConstantIndex_++;

		return currId;
	}

	void AsmBuilder::WriteInstruction(uint32 instr)
	{
		isEmpty_ = false;

		buffer_.push_back(instr);
	}

	void AsmBuilder::ClearState()
	{
		isEmpty_ = true;

		buffer_.clear();
		constantTable_.clear();
		constantCaches_.clear();
		functions_.clear();
		funcIndices_.clear();

		currentFunc_.name = "";
		currentFunc_.offset = 0;

		entryFuncIdx_ = -1;

		currentConstantIndex_ = 0;
	}

	void AsmBuilder::WriteMetadata(ubyte** sectionPtr, uint32& size)
	{
		MetadataHeader header;
		header.functionsCount = functions_.size();

		uint32 metadataSize = 0;

		for (auto& func : functions_)
		{
			metadataSize += sizeof(uint16) + func.name.size() + 1 + sizeof(uint32);
		}

		ASSERT(metadataSize != 0);

		metadataSize += sizeof(MetadataHeader);

		ubyte* ptr = new ubyte[metadataSize];

		// пишем header
		memcpy(ptr, &header, sizeof(MetadataHeader));

		ubyte* funcOffset = ptr + sizeof(MetadataHeader);

		for (auto& func : functions_)
		{
			uint16 nameLen = func.name.size();

			// копируем длину
			*((uint16*)funcOffset) = nameLen;

			// копируем имя
			memcpy(funcOffset + sizeof(uint16), func.name.c_str(), nameLen);

			*(funcOffset + sizeof(uint16) + nameLen) = '\0';

			// пишем оффсет
			*((uint32*)(funcOffset + sizeof(uint16) + nameLen + 1)) = func.offset;

			funcOffset += sizeof(uint16) + nameLen + 1 + sizeof(uint32);
		}

		*sectionPtr = ptr;
		size = metadataSize;
	}

	void AsmBuilder::WriteFunction(std::string& name)
	{
		FunctionInfo info{ .name = name, .offset = static_cast<uint32>(buffer_.size()) };

		currentFunc_ = info;

		functions_.push_back(info);

		funcIndices_.insert({ name, functions_.size() - 1 });

		isEmpty_ = false;
	}

	void AsmBuilder::SetEntryFunction(std::string& name)
	{
		auto funcOffset = funcIndices_.find(name);

		if (funcOffset == funcIndices_.end())
		{
			throw std::exception();
		}

		entryFuncIdx_ = (*funcOffset).second;
	}

	void AsmBuilder::WriteMov(uint32 dst, uint32 src, uint8 flags)
	{
		uint32 result = 0;

		result = static_cast<uint8>(InstructionType::eMov);

		result = (flags << 8) | (result & ~0x300);

		uint16 dstIndex = WriteConstant(dst);

		result = (dstIndex << 10) | (result & ~0xffc00);

		uint16 srcIndex = WriteConstant(src);

		result = (srcIndex << 20) | (result & ~0x3ff00000);

		WriteInstruction(result);
	}

	void AsmBuilder::WriteAdd(uint32 dst, uint32 src)
	{
		uint32 result = 0;

		result = static_cast<uint8>(InstructionType::eAdd);

		uint16 dstIndex = WriteConstant(dst);

		result = (dstIndex << 8) | (result & ~0x3ff00);

		uint16 srcIndex = WriteConstant(src);

		result = (srcIndex << 18) | (result & ~0xffc0000);

		WriteInstruction(result);
	}

	void AsmBuilder::WriteCall(std::string& funcName)
	{
		uint32 result = 0;

		result = static_cast<uint8>(InstructionType::eCall);

		auto funcOffset = funcIndices_.find(funcName);

		if (funcOffset == funcIndices_.end())
		{
			throw std::exception();
		}

		FunctionInfo& info = functions_[(*funcOffset).second];

		uint16 offset = info.offset;

		result = (offset << 8) | (result & ~0x3ff00);

		WriteInstruction(result);
	}

	void AsmBuilder::WriteBreakPoint()
	{
		uint32 result = 0;

		result |= static_cast<uint8>(InstructionType::eBrkPoint) & 0x0f;

		WriteInstruction(result);
	}

	ImageInfo AsmBuilder::GetImage()
	{
		if (isEmpty_)
		{
			throw std::exception();
		}

		if (entryFuncIdx_ == -1)
		{
			throw std::exception();
		}

		uint32 codeSize = buffer_.size() * sizeof(uint32);

		uint16 constCount = constantTable_.size();
		uint32 tableSize = constantTable_.size() * sizeof(uint32);

		ubyte* metadataPtr = nullptr;
		uint32 metadataSize = 0;

		WriteMetadata(&metadataPtr, metadataSize);

		ubyte* output = new ubyte[codeSize + tableSize + metadataSize + sizeof(SLMHeader)];

		SLMHeader header;
		header.codeSize = codeSize;
		header.constantTableCount = constCount;
		header.metadataSize = metadataSize;
		header.entryFunctionIdx = entryFuncIdx_;

		// пишем header
		memcpy(output, &header, sizeof(SLMHeader));

		// пишем metadata
		memcpy(output + sizeof(SLMHeader), metadataPtr, metadataSize);

		// пишем code
		memcpy(output + sizeof(SLMHeader) + metadataSize, buffer_.data(), codeSize);

		// пишем constants
		memcpy(output + sizeof(SLMHeader) + metadataSize + codeSize, constantTable_.data(), tableSize);

		ImageInfo info{};
		info.ptr = output;
		info.size = codeSize + metadataSize + tableSize + sizeof(SLMHeader);

		ClearState();

		delete[] metadataPtr;

		return info;
	}

}

