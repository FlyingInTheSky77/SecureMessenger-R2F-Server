#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "../include/BackEnd.h"
#include "../include/TcpDumpManager.h"

int main( int argc, char *argv[] )
{
    QCoreApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QGuiApplication app( argc, argv );

    qmlRegisterType< BackEnd >( "io.qt.BackEnd", 1, 0, "BackEnd" );
    qmlRegisterType< TcpDumpManager >( "io.qt.TcpDumpManager", 1, 0, "TcpDumpManager" );
    QQmlApplicationEngine engine;

    const QUrl url( QStringLiteral( "qrc:/qml/main.qml" ) );
    QObject::connect( &engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url]( QObject *obj, const QUrl &objUrl ) {
        if ( !obj && url == objUrl )
            QCoreApplication::exit( -1 );
    }, Qt::QueuedConnection );
    engine.load( url );

    return app.exec();
}
