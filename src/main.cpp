#include <QCoreApplication>
#include <QCommandLineParser>

#include "../include/BackEnd.h"
#include "../include/Logger.h"
#include "../include/TcpDumpManager.h"

int main( int argc, char *argv[] )
{
    QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QCoreApplication app(argc, argv);

    // Create the command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Test QCommandLineParser");
    parser.addHelpOption(); // Adds --help option by default

    // Define options (command-line flags)
    QCommandLineOption noLogOption(QStringList() << "n" << "no-log", "Disable logging.");
    QCommandLineOption consoleLogOption(QStringList() << "c" << "console-log", "Log to console.");
    QCommandLineOption logFileOption(QStringList() << "f" << "file-log", "Log to file.");

    // Add options to the parser
    parser.addOption(noLogOption);
    parser.addOption(consoleLogOption);
    parser.addOption(logFileOption);

    // Process the actual command-line arguments given by the user
    parser.process(app);

    std::shared_ptr< ILogger > logger;

    // Handle the command-line options
    if (parser.isSet(noLogOption)) {
        qDebug() << "Logging is disabled.";
    } else if (parser.isSet(consoleLogOption)) {
        qDebug() << "Logging to console.";
        logger = std::make_shared< ConsoleLogger >();
    } else if (parser.isSet(logFileOption)) {
        qDebug() << "Logging to file (default).";
        logger = std::make_shared< FileLogger > ( "/var/log/myapp/application.log" );
    } else {
        qDebug() << "No specific logging option set. Logging is disabled.";
    }

    BackEnd backEnd( logger );

    return app.exec();
}
