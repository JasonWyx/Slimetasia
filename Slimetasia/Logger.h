#pragma once
#include <direct.h>

#include <algorithm>
#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>

#ifdef USE_PASSERT
#define p_assert(expr) Logger::LogAssert((!!(expr)), __FILE__, __func__, __LINE__)
#else
#define p_assert(expr) true
#endif

namespace Logger
{
    void LogAssert(bool expr, const char* cpp, const char* func, long line);
}
