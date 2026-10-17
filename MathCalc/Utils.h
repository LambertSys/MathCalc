#pragma once

#include <exception>

#define ASSERT(equal) if(!(equal)) { __debugbreak(); std::terminate(); }