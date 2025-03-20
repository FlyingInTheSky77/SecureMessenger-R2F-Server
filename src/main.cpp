#include "BackEnd.h"
#include "CommandLineOptions.h"
#include "Logger.h"
#include "TcpDumpManager.h"

#include <QCoreApplication>
#include <QCommandLineParser>

int main( int argc, char *argv[] )
{
    QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QCoreApplication app(argc, argv);

    CommandLineOptions options;
    options.parse();

    std::shared_ptr< ILogger > logger = options.createLogger();

    BackEnd backEnd( logger );

    return app.exec();
}
