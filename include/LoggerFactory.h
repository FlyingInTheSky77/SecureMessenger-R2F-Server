#pragma once

#include "CommandLineOptions.h"
#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>

class LoggerFactory {
public:
    static std::shared_ptr<ILogger> getLogger();
    static void setLoggerType(LogOption type);

private:
    static LogOption& getLoggerType();
    static LogOption logType_;
};

class LogStream {
public:
    LogStream(const LogLevel level, const char* file, const char* function, const int line);
    ~LogStream();

    template<typename T>
    LogStream& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

private:
    LogLevel level_;
    std::ostringstream stream_;
};

#define logError   LogStream(LogLevel::ERROR, __FILE__, __FUNCTION__, __LINE__)
#define logWarning LogStream(LogLevel::WARNING, __FILE__, __FUNCTION__, __LINE__)
#define logInfo    LogStream(LogLevel::INFO, __FILE__, __FUNCTION__, __LINE__)
#define logDebug   LogStream(LogLevel::DEBUG, __FILE__, __FUNCTION__, __LINE__)
