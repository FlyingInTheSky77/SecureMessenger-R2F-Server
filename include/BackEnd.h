#pragma once

#include "Logger.h"
#include "LoggerFactory.h"
#include "ServerManager.h"

class BackEnd : public QObject
{
    Q_OBJECT

public:
    explicit BackEnd( QObject *parent = nullptr );
    ~BackEnd();

public slots:
    void smbConnectedToServer();
    void smbDisconnectedFromServer();
    void gotNewMesssage( QString message );

private:
    std::unique_ptr< ServerManager > serverManager_;
};
