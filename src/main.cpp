#include "BackEnd.h"
#include "CommandLineOptions.h"
#include "LoggerFactory.h"
#include "TcpDumpManager.h"

#include <QCoreApplication>
#include <QCommandLineParser>

int main( int argc, char *argv[] )
{
    QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QCoreApplication app(argc, argv);

    CommandLineOptions options;
    options.parse();

    LoggerFactory::setLoggerType(options.getLogOption());
    logInfo << "R2F-MessengerServer started";

    BackEnd backEnd;

    return app.exec();
}
