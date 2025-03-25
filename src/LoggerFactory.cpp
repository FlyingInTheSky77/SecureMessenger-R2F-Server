#include "LoggerFactory.h"

#include <filesystem>

std::shared_ptr<ILogger> LoggerFactory::getLogger() {
    if (logType_ == LogOption::NoLog) {
        return NullLogger::instance();
    } else if (logType_ == LogOption::FileLog) {
        return FileLogger::instance();
    } else if (logType_ == LogOption::ConsoleLog) {
        return ConsoleLogger::instance();
    } else {
        return NullLogger::instance();
    }
}

void LoggerFactory::setLoggerType(LogOption type) {
    getLoggerType() = type;
}

LogOption& LoggerFactory::getLoggerType() {
    return logType_;
}

LogOption LoggerFactory::logType_ = LogOption::FileLog;

LogStream::LogStream(const LogLevel level, const char* file, const char* function, const int line)
    : level_(level)
{
    std::string filename = std::filesystem::path(file).filename().string();
    stream_ << getLogTimestamp() << filename << "  " << function << "  "<< line << "  ";
}

LogStream::~LogStream() {
    LoggerFactory::getLogger()->log(level_, stream_.str());
}
