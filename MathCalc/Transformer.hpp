#pragma once

#include "AST.hpp"

namespace ast
{
	class ITransfomer
	{
	public:
		virtual void Transform(Context& context) = 0;
	};
}