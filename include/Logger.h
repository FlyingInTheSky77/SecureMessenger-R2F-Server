#pragma once

#include <string>
#include <iostream>
#include <fstream>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, const std::string& message) = 0;
    std::string logLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }
};

class ConsoleLogger : public ILogger {
public:
    void log(LogLevel level, const std::string& message) override {
        std::string levelStr = logLevelToString(level);
        std::lock_guard lock(mutex_);
        std::cout << "[" << levelStr << "] " << message << std::endl;
    }

private:
    std::mutex mutex_;
};

class FileLogger : public ILogger {
public:
    explicit FileLogger(const std::string& filename) : file(filename, std::ios::app) {
        file.open(filename, std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error opening log file: " << filename << std::endl;
        }
    }

    void log(LogLevel level, const std::string& message) override {
        if (file.is_open()) {
            std::string levelStr = logLevelToString(level);
            std::lock_guard lock(mutex_);
            file << "[" << levelStr << "] " << message << std::endl;
        }
        else {
            std::cerr << "Error opening log file " << std::endl;
        }
    }

private:
    std::ofstream file;
    std::mutex mutex_;
};


