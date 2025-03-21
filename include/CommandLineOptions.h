#pragma once

#include "Logger.h"

#include <QCommandLineParser>

#include <memory>

enum LogOption { NoLog, ConsoleLog, FileLog, NoOption };

class CommandLineOptions {
public:
    explicit CommandLineOptions();
    void parse();
    LogOption getLogOption();

private:
    QCommandLineParser parser_;
    LogOption logOption_;
};
