#include "Logger.h"

void Logger::LogAssert(bool expr, const char* cpp, const char* func, long line)
{
    if (!expr)
    {
        int result = _mkdir("LOG");

        char buf[100];
        std::ofstream myfile;
        time_t now = time(0);
        std::string filename = "LOG\\ASSERT_LOG_";
        ctime_s(buf, sizeof(buf), &now);
        std::string clock = buf;
        std::replace(clock.begin(), clock.end(), ':', '_');
        clock.pop_back();
        filename += clock;
        filename += ".txt";
        myfile.open(filename);
        if (myfile.is_open())
        {
            myfile << "File Name: " << cpp << std::endl;
            myfile << "Function Name: " << func << std::endl;
            myfile << "Line Number: " << line << std::endl;
        }
        myfile.close();
        assert(expr);
    }
}
