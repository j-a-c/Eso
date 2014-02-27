#ifndef ESO_LOGGER_LOGGER
#define ESO_LOGGER_LOGGER

#include <iostream>
#include <fstream> 
#include <string>
#include <time.h>
#include "../global_config/types.h"

enum class LogLevel {Fatal, Error, Warning, Info, Debug, Debug1};

/* Class for logging */
class Logger
{
public:
    static void log(std::string msg, LogLevel level = LogLevel::Info);
    static void log(uchar_vec msg, LogLevel level = LogLevel::Info);

private: 
    Logger();
    ~Logger();
};


// TODO generalize output path and add logging level
// TODO integrity checks
/**
 * Logs the message to the output location.
 */
void Logger::log(std::string msg, LogLevel level)
{
    // The logger output location.
    // TODO Configure.
    std::string log_loc{"/home/bose/Desktop/eso/default.log"};

    std::ofstream out;
    out.open(log_loc, 
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

/**
 * Delegates to another logging method.
 * Added to support logging char_vec's.
 */
void Logger::log(uchar_vec msg, LogLevel level)
{
    Logger::log(to_string(msg), level);
}

#endif
