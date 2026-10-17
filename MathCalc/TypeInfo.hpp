#pragma once

#include "DataType.hpp"

namespace ast
{
	enum class TypeKind
	{
		Base,
		Struct,
		Function
	};

	class FunctionDeclNode;

	struct TypeInfo
	{
		TypeKind kind = TypeKind::Base;

		bool isRef;

		DataType type;

		FunctionDeclNode* decl = nullptr;
	};
}
