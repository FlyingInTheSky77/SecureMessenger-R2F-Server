#include "CommandLineOptions.h"

#include <QDebug>

CommandLineOptions::CommandLineOptions() {
    parser_.setApplicationDescription("QCommandLineParser");
    parser_.addHelpOption();

    parser_.addOption({{"n", "no-log"}, "Disable logging."});
    parser_.addOption({{"c", "console-log"}, "Log to console."});
    parser_.addOption({{"f", "file-log"}, "Log to file."});

    parser_.process(QCoreApplication::arguments());
}

void CommandLineOptions::parse() {
    const bool noLog = parser_.isSet("no-log");
    const bool consoleLog = parser_.isSet("console-log");
    const bool fileLog = parser_.isSet("file-log");

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

LogOption CommandLineOptions::getLogOption()
{
    return logOption_;
}
