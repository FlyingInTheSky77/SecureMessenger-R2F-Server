#pragma once

#include "stdafx.h"

#include "ServerManager.h"
#include "Logger.h"

class BackEnd : public QObject
{
    Q_OBJECT

public:
    explicit BackEnd( std::shared_ptr< ILogger > logger = nullptr, QObject *parent = nullptr );
    ~BackEnd();

public slots:
    void smbConnectedToServer();
    void smbDisconnectedFromServer();
    void gotNewMesssage( QString message );

private:
    std::unique_ptr< ServerManager > serverManager_;
    std::shared_ptr< ILogger > logger_;
};
