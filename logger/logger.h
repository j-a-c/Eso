#ifndef ESO_LOGGER_LOGGER
#define ESO_LOGGER_LOGGER

#include <iostream>
#include <fstream> 
#include <time.h>

enum class LogLevel {Error, Warning, Info, Debug, Debug1};

/* Class for logging */
class Logger
{
public:
    static void log(std::string msg, LogLevel level = LogLevel::Info);
private: 
    Logger();
    ~Logger();
};

// TODO generalize output path and add logging level
// TODO integrity checks
void Logger::log(std::string msg, LogLevel level)
{
    std::ofstream out;
    out.open("/home/bose/Desktop/eso/default.log", 
            std::ios_base::app | std::ios_base::in | std::ios_base::out);

    // Formatted time
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char time_buffer [80];
    strftime (time_buffer, 80, "%F %r:\t", timeinfo);

    out << time_buffer 
        << msg << std::endl;
    out.close();
}

#endif
