#pragma once
#include "stdafx.h"
#include "server.h"

class BackEnd : public QObject
{
    Q_OBJECT

public:
    explicit BackEnd( QObject *parent = nullptr );

signals:
    void smbConnected();
    void smbDisconnected();
    void newMessage( QString message );

public slots:
    QString stopClicked();
    QString startClicked();
    QString testClicked();

    void smbConnectedToServer();
    void smbDisconnectedFromServer();
    void gotNewMesssage( QString message );

private:
    std::unique_ptr< Server > server_;
};
