#pragma once

#include <string>

#include "DataType.hpp"

/**
* @brief Возвращает число-ранк типа, по которому можно решить, что тип находится ниже или выше другого типа по приоритету при приведении типов
*/
int GetRank(DataType type);

bool IsUnsigned(DataType type);

std::string DataTypeToString(DataType type);