#include "BackEnd.h"
#include "CommandLineOptions.h"
#include "LoggerFactory.h"
#include "TcpDumpManager.h"

#include <QCoreApplication>
#include <QCommandLineParser>

#include <csignal>

QCoreApplication* app_ptr = nullptr;

void signalHandler(int signum) {
    qDebug() << "\n[Shutdown log] Caught signal " << signum << ". Initiating shutdown.";
    if (app_ptr) {
        app_ptr->quit();
    }
}

int main( int argc, char *argv[] )
{
    QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QCoreApplication app(argc, argv);
    app_ptr = &app;

    std::signal(SIGINT,  signalHandler);  // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // kill, systemctl stop
    std::signal(SIGHUP,  signalHandler);  // terminal closing

    CommandLineOptions options;
    options.parse();

    LoggerFactory::setLoggerType(options.getLogOption());
    logInfo << "R2F-MessengerServer started";

    BackEnd backEnd;

    return app.exec();
}
