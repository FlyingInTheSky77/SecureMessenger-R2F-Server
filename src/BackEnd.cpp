#include "include/server.h"
#include "include/BackEnd.h"

BackEnd::BackEnd( QObject *parent )
    : QObject{ parent }
    , server_{ std::make_unique< Server >() }
{
    connect( server_.get(), &Server::gotNewMesssage, this, &BackEnd::gotNewMesssage );
    connect( server_->tcpServer_.get(), &QTcpServer::newConnection, this, &BackEnd::smbConnectedToServer );
    connect( server_.get(), &Server::smbDisconnected, this, &BackEnd::smbDisconnectedFromServer );
}

QString BackEnd::startClicked()
{
    if ( !server_->tcpServer_->listen( QHostAddress::Any, 5555 ) )
    {
        return tr( "Error: port is taken by some other service" );
    }
    connect( server_->tcpServer_.get(), &QTcpServer::newConnection, server_.get(), &Server::newConnection );
    return tr( "Server started, port is openned");
}

QString BackEnd::stopClicked()
{
    if( server_->tcpServer_->isListening() )
    {
        server_->serverStoped();

        disconnect( server_->tcpServer_.get(), &QTcpServer::newConnection, server_.get(), &Server::newConnection );

        return tr( "Server stopped, port is closed" );
    }
    return tr( "Error: Server was not running");
}

QString BackEnd::testClicked()
{
    if( server_->tcpServer_->isListening() )
    {
        return QString( "%1 %2" )
               .arg( tr( "Server is listening, number of connected clients:" ) )
               .arg( QString::number( server_->getClients().count() ) );
    }
    return QString( "%1 %2" )
           .arg( tr( "Server is not listening, number of connected clients:") )
           .arg( QString::number( server_->getClients().count() ) );
}

void BackEnd::smbConnectedToServer()
{
    emit smbConnected();
}

void BackEnd::smbDisconnectedFromServer()
{    
    emit smbDisconnected();
}

void BackEnd::gotNewMesssage( QString message )
{
    emit newMessage( message );
}
