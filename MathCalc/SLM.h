#pragma once

#include <string>

namespace slm
{
	/**
	* @brief Stack Language Machine (SLM)
	*/

	/*
	* instruction len - 4 byte
	* 
	*/

	using uint8 = unsigned char;
	using uint16 = unsigned short;
	using uint32 = unsigned int;
	using uint64 = unsigned long long;

	using ubyte = unsigned char;

	enum InstructionType : uint8
	{
		eAdd = 0,
		eAddInt32,
		eAddInt16,
		eAddFloat32,
		eSub,
		eSubInt32,
		eSubInt16,
		eSubFloat32,
		eMulInt32,
		eMulInt16,
		eDivInt32,
		eDivInt16,
		eDivUInt32,
		eDivUInt16,
		eLoadConstInt32,
		eLoadConstUInt32,
		eLoadConstFloat32,
		eMov,
		eMov32,
		eMov16,
		eJmp,
		eCmp,
		eCall,
		eRet,
		eBrkPoint,
		eAssert
	};

	namespace mov_flags
	{
		constexpr uint8 USE_STACK_SRC = 0x0;
		constexpr uint8 USE_CONSTANT_SRC = 0x1;
	}

	enum class VMStatus
	{
		Finish,
		Error,
		Assert
	};

	constexpr int INSTRUCTION_SIZE = 4;

	/*
	* -- формат хедера
	* int32 - размер код секции в байтах
	* int16 - кол-во констант в таблице констант
	* int32 - размер секции метаданных
	* int32 - номер entry функции в таблице функций
	* -- Секция метаданных
	* int32 - количество функций
	* Таблица функций
	* uint16 - длина имени функции, потом null-terminated имя, потом int32 смещение в код-секции, чтобы начать выполнение
	*  -- тут начинается код-секция
	*  -- таблица констант начинается сразу после код-секции
	*/
#pragma pack(push, 1)
	struct SLMHeader
	{
		uint32 codeSize = 0;
		uint16 constantTableCount = 0;
		uint32 metadataSize = 0;
		uint32 entryFunctionIdx = 0;
	};
#pragma pack(pop)

	struct MetadataHeader
	{
		uint32 functionsCount = 0;
	};

	struct FunctionInfo
	{
		std::string name;
		uint32 offset;
	};
}