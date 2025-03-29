#include "ConnectedClientManager.h"

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
    auto it = std::find_if(clients_.begin(), clients_.end(),
                           [&login](const auto &pair) { return pair.login == login; });

    return (it != clients_.end()) ? it.key() : nullptr;
}

int ConnectedClientManager::getNumberConnectedClients() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return clients_.count();
}

QList < QTcpSocket* > ConnectedClientManager::getClientSocketsList() const
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
            it.key()->deleteLater();
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

std::optional< int > ConnectedClientManager::getClientIntermediateNumber( QTcpSocket *socket ) const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    auto it = clients_.find(socket);
    if (it != clients_.end())
    {
        return it.value().session_key_object.getIntermediateNumberForClient();
    }
    return std::nullopt;
}

int ConnectedClientManager::getUserSessionKey( QTcpSocket *socket ) const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    auto it = clients_.constFind(socket);
    if (it != clients_.cend())
    {
        qDebug() << __FILE__ << __LINE__ << " user_info.login: " << it->login;
        auto user_info = clients_[socket];
        return it->session_key_object.getSessionKey();
    }

    qDebug() << __FILE__ << __LINE__ << " Error: the socket not found";
    return -1;
}

void ConnectedClientManager::removeUser( QTcpSocket *disconnected_user_socket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if (clients_.remove( disconnected_user_socket ))
    {
        qDebug() << __FILE__ << __LINE__  << "remove user";
        disconnected_user_socket->close();
        disconnected_user_socket->deleteLater();
    }
    else
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't remove client from Qmap clients_: socket doen't match";
    }
}
