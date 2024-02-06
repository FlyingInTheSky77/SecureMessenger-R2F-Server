#pragma once

#include "stdafx.h"

#include "ServerManager.h"

class BackEnd : public QObject
{
    Q_OBJECT

public:
    explicit BackEnd( QObject *parent = nullptr );
    ~BackEnd();

    // called from the qml layer:
    Q_INVOKABLE QString stopClicked();
    Q_INVOKABLE QString startClicked();
    Q_INVOKABLE QString testConnectionClicked();

signals:
    void smbConnected_signal();
    void smbDisconnected_signal();
    void newMessage_signal( QString message );

public slots:
    void smbConnectedToServer();
    void smbDisconnectedFromServer();
    void gotNewMesssage( QString message );

private:
    std::unique_ptr< ServerManager > serverManager_;
};
