#pragma once

#include "server.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <vector>

#include <QJsonObject>
#include <QTcpSocket>

class ServerManager: public QObject
{
    Q_OBJECT

public:
    ServerManager();
    ~ServerManager();
    QString startServer();
    QString stopServer();
    QString showServerStatus();
    std::optional < std::pair<QJsonObject, QTcpSocket *> > dequeueMessage();
    void stop();

public slots:
    void enqueueMessage( QJsonObject obj, QTcpSocket* clientSocket );    

signals:
    void smbConnected_signal();
    void gotNewMesssage_signal( QString msg );
    void smbDisconnected_signal();

private:
    void threadFunction();

    std::unique_ptr< Server > server_;
    std::queue< std::pair< const QJsonObject, QTcpSocket* > > messageQueue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_flag_;
    std::vector< std::thread > threads_;
};
