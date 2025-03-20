#pragma once

#include "Logger.h"

#include <QCommandLineParser>

#include <memory>

enum LogOption { NoLog, ConsoleLog, FileLog, NoOption };

class CommandLineOptions {
public:
    explicit CommandLineOptions();
    void parse();
    std::shared_ptr<ILogger> createLogger() const;

private:
    QCommandLineParser parser;
    LogOption logOption_;
};
