#pragma once

#include "server.h"

#include <queue>

class ServerManager: public QObject
{
    Q_OBJECT

public:
    ServerManager();
    ~ServerManager();
    QString startServer();
    QString stopServer();
    QString showServerStatus();

public slots:
    void enqueueMessage( QJsonObject obj, QTcpSocket* clientSocket );

signals:
    void smbConnected_signal();
    void gotNewMesssage_signal( QString msg );
    void smbDisconnected_signal();

private:
    std::unique_ptr< Server > server_;
    std::queue< std::pair< const QJsonObject, QTcpSocket* > > messageQueue_;
};
