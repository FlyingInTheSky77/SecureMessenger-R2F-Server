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
    std::optional< int > getClientIntermediateNumber( QTcpSocket *socket ) const;
    int getUserSessionKey( QTcpSocket *socket ) const;

    int getNumberConnectedClients() const;
    QString getUserLogin( QTcpSocket *socket );
    QTcpSocket* getSocketIfUserIsAuthorized( const QString& login );
    QList < QTcpSocket* > getClientSocketsList() const;

    void removeUser( QTcpSocket* disconnected_user_socket );
    void clearClients();

private:
    QMap< QTcpSocket*, ClientsSessionsInfo > clients_;
    mutable std::mutex mutex_;
};
