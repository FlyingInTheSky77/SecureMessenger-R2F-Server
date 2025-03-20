#include "CommandLineOptions.h"

#include <QDebug>

CommandLineOptions::CommandLineOptions() {
    parser.setApplicationDescription("QCommandLineParser");
    parser.addHelpOption();

    parser.addOption({{"n", "no-log"}, "Disable logging."});
    parser.addOption({{"c", "console-log"}, "Log to console."});
    parser.addOption({{"f", "file-log"}, "Log to file."});

    parser.process(QCoreApplication::arguments());
}

void CommandLineOptions::parse() {
    const bool noLog = parser.isSet("no-log");
    const bool consoleLog = parser.isSet("console-log");
    const bool fileLog = parser.isSet("file-log");

    const int8_t selectedOptions = noLog + consoleLog + fileLog;

    if (selectedOptions == 0) {
        logOption_ = LogOption::NoOption;
    }
    else if (noLog == true && selectedOptions > 1) {
        qDebug() << "You selected no logging and another logging type, "
                    "the no logging type will be set.";
        logOption_ = LogOption::NoLog;
    }
    else if (selectedOptions > 1 ) {        
        qDebug() << "You have selected several types of logging, "
                    "so only file logging will be selected as default.";
        logOption_ = LogOption::FileLog;
    }
    else if (fileLog) {
        qDebug() << "Logging to file.";
        logOption_ = LogOption::FileLog;
    }
    else if (consoleLog) {
        qDebug() << "Logging to console.";
        logOption_ = LogOption::ConsoleLog;
    }
    else if (noLog) {
        qDebug() << "Logging is disabled.";
        logOption_ = LogOption::NoLog;
    }
    else {
        qDebug() << "No specific logging option set. Logging is disabled.";
    }
}

std::shared_ptr<ILogger> CommandLineOptions::createLogger() const {
    if (logOption_ == LogOption::NoLog) {
        return nullptr;
    } else if (logOption_ == LogOption::FileLog) {        
        return std::make_shared<FileLogger>();
    } else if (logOption_ == LogOption::ConsoleLog) {
        return std::make_shared<ConsoleLogger>();
    } else {
        return nullptr;
    }
}
