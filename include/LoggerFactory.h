#pragma once

#include "Logger.h"

#include <memory>

class LoggerFactory {
public:
    static std::shared_ptr<ILogger> getLogger() {
        if (logOption_ == LogOption::NoLog) {
            return NullLogger::instance();
        } else if (logOption_ == LogOption::FileLog) {
            return FileLogger::instance();
        } else if (logOption_ == LogOption::ConsoleLog) {
            return ConsoleLogger::instance();
        } else {
            return NullLogger::instance();
        }
    }

    static void setLoggerType(LogOption type) {
        getLoggerType() = type;
    }

private:
    static LogOption& getLoggerType() {
        return logOption_;
    }
    static LogOption logOption_;
};

LogOption LoggerFactory::logOption_ = LogOption::FileLog;

void log(LogLevel level, const std::string& message) {
    static std::shared_ptr<ILogger> logger = LoggerFactory::getLogger();
    logger->log(level, message);
}
