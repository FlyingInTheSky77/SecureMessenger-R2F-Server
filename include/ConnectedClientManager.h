#pragma once

#include "SessionKey.h"

#include <QList>
#include <QTcpSocket>

#include <mutex>
#include <optional>

struct ClientsSessionsInfo
{
    SessionKey session_key_object;
    QString login;
};

class ConnectedClientManager
{
public:
    void addNewClientSocket( QTcpSocket* socket );
    bool saveUserName( const QString& login, QTcpSocket *ClientSocket );

    void calculateAndSaveClientSessionKey( int number_from_client, QTcpSocket *socket );
    std::optional< int > getClientIntermediateNumber( QTcpSocket *socket );
    int getUserSessionKey( QTcpSocket *socket );

    int getNumberConnectedClients();
    QString getUserLogin( QTcpSocket *socket );
    QTcpSocket* getSocketIfUserIsAuthorized( const QString& login );
    QList < QTcpSocket* > getClientSocketsList();

    void removeUser( QTcpSocket* disconnected_user_socket );
    void clearClients();

private:
    QMap< QTcpSocket*, ClientsSessionsInfo > clients_;
    std::mutex mutex_;
};
