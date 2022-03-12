#include "Logger.h"

void Logger::LogAssert(bool expr, const char* cpp, const char* func, long line)
{
    if (!expr)
    {
        int result = _mkdir("Logs");

        time_t now = time(0);

        char timeStamp[100];
        ctime_s(timeStamp, sizeof(timeStamp), &now);

        std::ofstream outFile;
        std::string filename = "Logs\\ASSERT_LOG.txt";

        std::string timeStampString = timeStamp;

        outFile.open(filename);
        if (outFile.is_open())
        {
            outFile << timeStampString << ":" << cpp << " " << func << ":" << line << std::endl;
        }
        outFile.close();

        assert(expr);
    }
}
