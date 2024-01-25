#include "include/Database.h"
#include "include/server.h"

#include <memory>

Server::Server()
    : QObject()
    , tcpServer_{ std::make_unique< QTcpServer >() }
    , messageProcessor_( std::make_unique < MessageProcessor >() )
{
    connect( messageProcessor_.get(), &MessageProcessor::sendToClient_signal, this, &Server::sendToClient );
    connect( messageProcessor_.get(), &MessageProcessor::showMessage_signal, this, &Server::showMessage );
}

QList< QTcpSocket* > Server::getClients()
{
    return messageProcessor_.get() -> getClientsList();
}

void Server::newConnection()
{
    QTcpSocket *clientSocket{ tcpServer_->nextPendingConnection() };

    connect( clientSocket, &QTcpSocket::readyRead, this, &Server::readClient );
    connect( clientSocket, &QTcpSocket::disconnected, this, &Server::gotDisconnection );

    messageProcessor_.get() -> addNewClientSocket( clientSocket );
}

void Server::disconnectSockets()
{
    auto clients = messageProcessor_.get() -> getClientsMap();

    for ( QMap<QTcpSocket*, client_struct>::iterator it = clients.begin(); it != clients.end(); ++it )
    {
        disconnect( it.key(), &QTcpSocket::readyRead, this, &Server::readClient );
        disconnect( it.key(), &QTcpSocket::disconnected, this, &Server::gotDisconnection );
    }

    messageProcessor_.get() -> clearClients();
}

void Server::readClient()
{
    QTcpSocket *clientSocket{ static_cast< QTcpSocket* >( sender() ) };
    QJsonParseError parseError;
    parseError.error = QJsonParseError::NoError;
    const QJsonDocument doc{ QJsonDocument::fromJson( clientSocket->readAll(), &parseError ) };
    if ( parseError.error )
    {
        emit gotNewMesssage( tr( "Error: QJsonParseError" ) );
        return;
    }
    const QJsonObject obj{ doc.object() };
    messageProcessor_.get() -> processIncomingMessages( obj, clientSocket );
}

void Server::showMessage( QString msg )
{
    emit gotNewMesssage( msg );
}

void Server::gotDisconnection()
{
    QTcpSocket* disconnected_user_socket = static_cast< QTcpSocket* >( sender() );

    messageProcessor_.get() -> processUserDisconnection(disconnected_user_socket);
    disconnected_user_socket -> destroyed();

    emit smbDisconnected();    
}

void Server::sendToClient( QTcpSocket *socket, const QByteArray jByte )
{
    qint64 bytesWritten = socket->write( jByte );
    if( bytesWritten == -1 )
    {
        qDebug() << "Error writing to socket:" << socket->errorString();
    }
    else
    {
        qDebug() << __FILE__ << __LINE__ << "Successfully wrote";
    }
}

void Server::serverStoped()
{
    messageProcessor_.get() -> sendClientsServerStoped();
    // Stop listen NEW clients, with try to connect:
    tcpServer_->close();
    // Stop listen already connected clients:
    disconnectSockets();
}
