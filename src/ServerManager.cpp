#include "include/ServerManager.h"

ServerManager::ServerManager()
    : QObject()
    , server_( std::make_unique< Server >() )
{
    connect( server_.get(), &Server::gotNewMesssage_signal, this, &ServerManager::gotNewMesssage_signal );
    connect( server_.get(), &Server::smbConnected_signal, this, &ServerManager::smbConnected_signal );
    connect( server_.get(), &Server::smbDisconnected_signal, this, &ServerManager::smbDisconnected_signal );

    connect( server_.get(), &Server::recivedMessageFromClient_signal, this, &ServerManager::enqueueMessage );
}

ServerManager::~ServerManager()
{
    disconnect( server_.get(), &Server::gotNewMesssage_signal, this, &ServerManager::gotNewMesssage_signal );
    disconnect( server_.get(), &Server::smbConnected_signal, this, &ServerManager::smbConnected_signal );
    disconnect( server_.get(), &Server::smbDisconnected_signal, this, &ServerManager::smbDisconnected_signal );

    disconnect( server_.get(), &Server::recivedMessageFromClient_signal, this, &ServerManager::enqueueMessage );
}

QString ServerManager::startServer()
{
    return server_->start();
}

QString ServerManager::stopServer()
{
    return server_->stop();
}

QString ServerManager::showServerStatus()
{
    return server_->showServerStatus();
}

void ServerManager::enqueueMessage( QJsonObject obj, QTcpSocket* clientSocket)
{
    messageQueue_.push( std::make_pair( obj, clientSocket ) );
}
