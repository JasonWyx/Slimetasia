#pragma once
#include <direct.h>

#include <algorithm>
#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>

#ifdef ENABLE_ASSERTS
#define ASSERT(expr) Logger::LogAssert((!!(expr)), __FILE__, __func__, __LINE__)
#else
#define ASSERT(expr) true
#endif

namespace Logger
{
    void LogAssert(bool expr, const char* cpp, const char* func, long line);
}
