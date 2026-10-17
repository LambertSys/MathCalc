#pragma once

#include <string>
#include <vector>

#include "DataType.hpp"

struct FunctionArg
{
	std::string name;
	DataType type;
};

class Function
{
private:
	std::string m_name;
	std::vector<FunctionArg> m_args;

	DataType m_retType;

public:
	void SetName(std::string name) { m_name = name; }
	std::string& GetName() { return m_name; }

	void AddArg(FunctionArg arg) { m_args.push_back(arg); }
	std::vector<FunctionArg>& GetArgs() { return m_args; }

	void SetReturnType(DataType retType) { m_retType = retType; }
	DataType GetReturnType() { return m_retType; }
};