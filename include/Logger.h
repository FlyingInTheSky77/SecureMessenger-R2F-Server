#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>

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

class NullLogger : public ILogger {
public:
    static std::shared_ptr<NullLogger> instance() {
        static std::shared_ptr<NullLogger> instance(new NullLogger());
        return instance;
    }

    void log(LogLevel, const std::string&) override {
        // no logging
    }
};

class ConsoleLogger : public ILogger {
public:
    static std::shared_ptr<ConsoleLogger> instance() {
        static std::shared_ptr<ConsoleLogger> logger(new ConsoleLogger());
        return logger;
    }
    void log(LogLevel level, const std::string& message) override {
        std::string levelStr = logLevelToString(level);
        std::lock_guard lock(mutex_);
        std::cout << "[" << levelStr << "] " << message << std::endl;
    }

private:
    ConsoleLogger() {}
    std::mutex mutex_;
};

class FileLogger : public ILogger {
public:
    static std::shared_ptr<FileLogger> instance() {
        static std::shared_ptr<FileLogger> logger(new FileLogger());
        return logger;
    }
    ~FileLogger() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    void log(LogLevel level, const std::string& message) override {
        std::lock_guard lock(mutex_);
        if (file_.is_open()) {
            std::string levelStr = logLevelToString(level);
            file_ << "[" << levelStr << "] " << message << std::endl;
            file_.flush();
        }
        else {
            std::cerr << "Error opening log file " << std::endl;
        }
    }
private:
    explicit FileLogger()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* local_time = std::localtime(&now_c);

        char timestamp[20];
        std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", local_time);
        std::string logFileName = "log_" + std::string(timestamp) + ".txt";

        file_.open(logFileName, std::ios::app);
        if (file_.is_open()) {
            std::lock_guard lock(mutex_);
            chmod(logFileName.c_str(), 0666);
            file_ << "Log started at " << timestamp << "\n";
            file_.flush();
        }
        else {
            std::cerr << "Error opening log file: " << logFileName << std::endl;
        }
    }

    std::ofstream file_;
    std::mutex mutex_;
};
