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

    QString start();
    QString stop();
    QString showServerStatus();

public slots:
    void gotDisconnection();

    void showMessage( QString msg );
    void sendToClient( QTcpSocket *socket, const QByteArray jByte );

signals:
    void smbConnected_signal();
    void gotNewMesssage_signal( QString msg );
    void smbDisconnected_signal();

private:
    std::unique_ptr< QTcpServer > tcpServer_;
    std::unique_ptr< MessageProcessor > messageProcessor_;

    virtual void newConnection();
    void readClient();
    void disconnectSockets();
};
