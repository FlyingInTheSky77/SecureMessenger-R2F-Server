#include "../include/Database.h"
#include "../include/server.h"

#include <memory>

Server::Server()
    : QObject()
    , tcpServer_{ std::make_unique< QTcpServer >() }
    , messageProcessor_( std::make_unique < MessageProcessor >() )
{
    connect( messageProcessor_.get(), &MessageProcessor::sendToClient_signal, this, &Server::sendToClient );
    connect( messageProcessor_.get(), &MessageProcessor::showMessage_signal, this, &Server::showMessage );
}

void Server::newConnection()
{
    QTcpSocket *clientSocket{ tcpServer_->nextPendingConnection() };

    connect( clientSocket, &QTcpSocket::readyRead, this, &Server::readClient );
    connect( clientSocket, &QTcpSocket::disconnected, this, &Server::gotDisconnection );

    messageProcessor_ -> addNewClientSocket( clientSocket );

    emit smbConnected_signal();
}

void Server::disconnectSockets()
{
    QList< QTcpSocket * > clientSocketsList = messageProcessor_->getClientSocketsList();

    for ( auto socket: clientSocketsList )
    {
        disconnect( socket, &QTcpSocket::readyRead, this, &Server::readClient );
        disconnect( socket, &QTcpSocket::disconnected, this, &Server::gotDisconnection );
    }

    messageProcessor_->clearClients();
}

QString Server::start()
{
    QString start_server_result;
    if ( !tcpServer_->listen( QHostAddress::Any, 5555 ) )
    {
        start_server_result = tr( "Error: port is taken by some other service" );
    }
    else
    {
        connect( tcpServer_.get(), &QTcpServer::newConnection, this, &Server::newConnection );
        start_server_result = tr( "Server started, port is openned");
    }

    return start_server_result;
}

void Server::readClient()
{
    QTcpSocket *clientSocket{ static_cast< QTcpSocket* >( sender() ) };
    QJsonParseError parseError;
    parseError.error = QJsonParseError::NoError;
    const QJsonDocument doc{ QJsonDocument::fromJson( clientSocket->readAll(), &parseError ) };
    if ( parseError.error )
    {
        emit gotNewMesssage_signal( tr( "Error: QJsonParseError" ) );
    }
    else
    {
        const QJsonObject obj{ doc.object() };
        emit recivedMessageFromClient_signal( obj, clientSocket ); // TODO implement subsequent logic in next steps
        messageProcessor_->processIncomingMessages( obj, clientSocket );
    }
}

void Server::showMessage( QString msg )
{
    emit gotNewMesssage_signal( msg );
}

void Server::gotDisconnection()
{
    QTcpSocket* disconnected_user_socket = static_cast< QTcpSocket* >( sender() );

    messageProcessor_->processUserDisconnection(disconnected_user_socket);
    disconnected_user_socket->destroyed();

    emit smbDisconnected_signal();
}

void Server::sendToClient( QTcpSocket *socket, const QByteArray jByte )
{
    qint64 bytesWritten = socket->write( jByte );
    if( bytesWritten == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error writing to socket:" << socket->errorString();
    }
    else
    {
        qDebug() << __FILE__ << __LINE__ << "Successfully wrote";
    }
}

QString Server::stop()
{
    QString stopServerResult;

    if( tcpServer_ -> isListening() )
    {
        messageProcessor_->sendClientsServerStoped();
        // Stop listen NEW clients, with try to connect:
        tcpServer_->close();
        // Stop listen already connected clients:
        disconnectSockets();

        disconnect( tcpServer_.get(), &QTcpServer::newConnection, this, &Server::newConnection );
        stopServerResult = tr( "Server stopped, port is closed" );
    }
    else
    {
        stopServerResult = tr( "Error: Server was not running" );
    }

    return stopServerResult;
}

QString Server::showServerStatus()
{
    const int number_connected_clients = messageProcessor_->getNumberConnectedClients();
    QString server_status;
    if( tcpServer_->isListening() )
    {
        server_status = QString( "%1 %2" )
                 .arg( tr( "Server is listening, number of connected clients:" ) )
                 .arg( QString::number( number_connected_clients ) );
    }
    else
    {
        server_status = QString( "%1 %2" )
                 .arg( tr( "Server is not listening, number of connected clients:") )
                 .arg( QString::number( number_connected_clients ) );
    }
    return server_status;
}
