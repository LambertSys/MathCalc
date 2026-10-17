#include "TypeUtils.h"

int GetRank(DataType type)
{
    switch (type)
    {
    case DataType::Bool:
        return 0;
    case DataType::Int32:
    case DataType::UInt32:
        return 1;
    }

    return -1;
}

bool IsUnsigned(DataType type)
{
    switch (type)
    {
    case DataType::UInt32:
        return true;
    }

    return false;
}

std::string DataTypeToString(DataType type)
{
    switch (type)
    {
    case DataType::Int32:
        return "int32";
    case DataType::UInt32:
        return "uint32";
    case DataType::Bool:
        return "bool";
    case DataType::Void:
        return "void";
    }

    return "Undefined";
}
