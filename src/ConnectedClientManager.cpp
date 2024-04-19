#include "../include/ConnectedClientManager.h"

void ConnectedClientManager::addNewClientSocket( QTcpSocket *socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    clients_.insert( socket, { {},{""} } );
}

bool ConnectedClientManager::saveUserName( const QString& login, QTcpSocket *socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( clients_.contains( socket ) )
    {
        clients_[ socket ].login = login;
        return true;
    }
    else
    {
        return false;
    }
}

QTcpSocket* ConnectedClientManager::getSocketIfUserIsAuthorized( const QString& login )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    QTcpSocket *recipient_socket = nullptr;

    for ( QMap<QTcpSocket*, ClientsSessionsInfo>::iterator it = clients_.begin(); it != clients_.end(); ++it )
    {
        if (it.value().login == login)
        {
            recipient_socket = it.key();
            break;
        }
    }
    return recipient_socket;
}

int ConnectedClientManager::getNumberConnectedClients()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return clients_.count();
}

QList < QTcpSocket* > ConnectedClientManager::getClientSocketsList()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return clients_.keys();
}

void ConnectedClientManager::clearClients()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( !clients_.isEmpty() ) {
        for ( auto it = clients_.begin(); it != clients_.end(); ++it )
        {
            delete it.key();
        }
        clients_.clear();
    }
}

QString ConnectedClientManager::getUserLogin( QTcpSocket *socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( clients_.contains( socket ) ) {
        return clients_[socket].login;
    }
    return {};
}

void ConnectedClientManager::calculateAndSaveClientSessionKey( int number_from_client, QTcpSocket *socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( clients_.contains( socket ) )
    {
        clients_[socket].session_key_object.calculateSessionKey( number_from_client );
    }
}

std::optional< int > ConnectedClientManager::getClientIntermediateNumber( QTcpSocket *socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( clients_.contains( socket ) )
    {
        return clients_[socket].session_key_object.getIntermediateNumberForClient();
    }
    else
    {
        return std::nullopt;
    }
}

int ConnectedClientManager::getUserSessionKey( QTcpSocket *socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( clients_.contains( socket ) )
    {
        auto user_info = clients_[socket];
        qDebug() << __FILE__ << __LINE__ << " user_info.login: " << user_info.login;

        return user_info.session_key_object.getSessionKey();
    }
    else
    {
        qDebug() << __FILE__ << __LINE__ << " Error getting SessionKey ";
        return -1;
    }
}

void ConnectedClientManager::removeUser( QTcpSocket *disconnected_user_socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if (clients_.contains( disconnected_user_socket ))
    {
        qDebug() << __FILE__ << __LINE__  << "remove user";
        clients_.remove( disconnected_user_socket );
    }
    else
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't remove client from Qmap clients_: socket doen't match";
    }
}
