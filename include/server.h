#pragma once

#include "stdafx.h"
#include "messagecode.h"
#include "MessageProcessor.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>

class Server: public QObject
{
    Q_OBJECT

public:
    Server();
    std::unique_ptr< QTcpServer > tcpServer_; // move it in private section in next step

    QList< QTcpSocket* > getClients();

    void disconnectSockets();
    void serverStoped();

public slots:
    virtual void newConnection();
    void readClient();
    void gotDisconnection();

    void showMessage( QString msg );
    void sendToClient( QTcpSocket *socket, const QByteArray jByte );

signals:
    void gotNewMesssage( QString msg );
    void smbDisconnected();

private:
    std::unique_ptr< MessageProcessor > messageProcessor_;
};
