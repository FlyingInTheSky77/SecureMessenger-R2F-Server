#include "include/server.h"
#include "include/Database.h"
#include <memory>

Server::Server()
    : QObject()
    , tcpServer_ { std::make_unique< QTcpServer >() }
{}

QList< QTcpSocket * > Server::getClients()
{
    QList< QTcpSocket * > clients_list;
    foreach( auto item, clients_ )
    {
        clients_list << item.socket;
    }
    return clients_list;
}

void Server::newConnection()
{
    QTcpSocket *clientSocket{ tcpServer_->nextPendingConnection() };

    connect( clientSocket, &QTcpSocket::readyRead, this, &Server::readClient );
    connect( clientSocket, &QTcpSocket::disconnected, this, &Server::gotDisconnection );

    const int number{ clients_.count() };
    clients_.insert( number, { clientSocket, {},{""} } );
    sendToClient( clientSocket, Server_Code::connection_established );
}

void Server::disconnectSockets()
{    
    for ( const auto& item: clients_ )
    {
        disconnect( item.socket, &QTcpSocket::readyRead, this, &Server::readClient );
        disconnect( item.socket, &QTcpSocket::disconnected, this, &Server::gotDisconnection );
    }
    clients_.clear();
}

void Server::readClient()
{
    QTcpSocket *clientSocket{ static_cast< QTcpSocket* >( sender() ) };
    QJsonParseError parseError;
    parseError.error =  QJsonParseError::NoError;
    const QJsonDocument doc{ QJsonDocument::fromJson( clientSocket->readAll(), &parseError ) };
    if ( parseError.error )
    {
        emit gotNewMesssage( tr( "Error: QJsonParseError" ) );
        return;
    }
    const QJsonObject obj{ doc.object() };
    letsSeeWhatWeGotFromClient( obj, clientSocket );
}

void Server::letsSeeWhatWeGotFromClient( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const int message_code{ obj.value( "code" ).toInt() };
    Client_Code message_client_code{ static_cast< Client_Code >( message_code )};
    switch ( message_client_code )
    {
        case Client_Code::message_to_server:
            readMessageToServer( obj, clientSocket );
            break;
        case Client_Code::registration_request:
            registrationRequest( obj,clientSocket );
            break;
        case Client_Code::authorization_request:
            authorizationRequest( obj, clientSocket );
            break;
        case Client_Code::contacts_request:
            sendToClientContactList( clientSocket );
            break;
        case Client_Code::message_to_recipient:
            readAndForwardMessageToRecipient( obj, clientSocket );
            break;
        case Client_Code::secure_session_client_step:
            secureSessionClientStep( obj, clientSocket );
            break;
        case Client_Code::mistake:
            receiveMistake( obj, clientSocket );
            break;
        default:
            emit gotNewMesssage( tr( "we got unknown message from client" ) );
            sendToClient( clientSocket, Server_Code::mistake, { { "message", tr( "you send to server unknown message-type" ) } } );
    }
}

void Server::receiveMistake( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    const QString message_to_server{ decrypt_object.value( "message" ).toString() };
    emit gotNewMesssage( tr( "mistake message from client: " ) + message_to_server);
}

void Server::readMessageToServer( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    const QString message_to_server{ decrypt_object.value( "message" ).toString() };
    emit gotNewMesssage( tr( "received ENCRYPTED message from client to server: " ) + encrypt_obj_inQString );
    emit gotNewMesssage( tr( "message after decryption: " ) + message_to_server );
}

void Server::secureSessionClientStep( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    int intermediate_number_from_client = obj.value( "first_step" ).toInt();
    calculateAndSetInClientMap( intermediate_number_from_client, clientSocket );

    if ( auto m_intermediate_number = getIntermediatNumberBySocketFromMap( clientSocket ); m_intermediate_number != std::nullopt )
    {
        const QJsonObject cur_object{ { "code", static_cast< int >( Server_Code::secure_session_server_step) }, { "second_step", m_intermediate_number.value() } };
        const QJsonDocument doc_encrypt( cur_object );
        const QByteArray jByte_encrypt( doc_encrypt.toJson( QJsonDocument::Compact ) );
        clientSocket->write( jByte_encrypt );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: getIntermediatNumberBySocketFromMap";
}

void Server::readAndForwardMessageToRecipient( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    if ( decrypt_object.empty() )
    {
        sendToClient( clientSocket, Server_Code::recipient_offline, { { "message", tr( "client offline for now, send message letter" ) } } );
        return;
    }
    const QString recipient{ decrypt_object.value( "recipient" ).toString() };
    const QString author{ decrypt_object.value( "author" ).toString() };
    if ( isRecipientconnectedNow( recipient ) )
    {
        if ( !forwardMessageToRecipient( recipient, decrypt_object ) )
        {
            qDebug() << __FILE__ << __LINE__ << "Error on server during forward message to" + recipient + " from" + author;
            emit gotNewMesssage( tr( "error forward during forward message to" ) + recipient + " from" + author );
        }
        return;
    }
    sendToClient( clientSocket, Server_Code::recipient_offline, { { "message", tr( "client offline for now, send message letter") } } );
}

void Server::registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    const QString login{ decrypt_object.value( "login" ).toString() };
    const QString password{ decrypt_object.value( "password" ).toString() };
    const QByteArray passwor_int_QByteArray{ password.toUtf8() };
    const QByteArray hash_of_password_for_database{ QCryptographicHash::hash( passwor_int_QByteArray, QCryptographicHash::Sha1 ) };
    const std::string pass_std{ hash_of_password_for_database.toStdString() };
    const QString pass_hashed_in_QString{ QString::fromUtf8( pass_std.c_str() ) };

    QString registration_result_in_text;
    if ( myDatabase_.registration( login, pass_hashed_in_QString, registration_result_in_text ) )
    {
        registration_result_in_text = login + tr( "success registration. You are in system");
        emit gotNewMesssage( tr( "registration was successful" ) );
        myDatabase_.setActivityStatus( login, 1 );
        sendToClient( clientSocket, Server_Code::registration_successful, { { "message", registration_result_in_text } } );
        return;
    }
    emit gotNewMesssage( tr( "registration with login" ) + login + tr( "was faild: " ) + registration_result_in_text );
    sendToClient( clientSocket, Server_Code::registration_successful, { { "message", registration_result_in_text } } );
}

void Server::authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    const QString login{ decrypt_object.value( "login" ).toString() };
    const QString password{ decrypt_object.value( "password" ).toString() };
    const QByteArray passwor_int_QByteArray{ password.toUtf8() };
    const QByteArray hash_of_password_for_database{ QCryptographicHash::hash( passwor_int_QByteArray, QCryptographicHash::Sha1 ) };
    QString authorization_result_in_text;
    if ( myDatabase_.authorization( login, hash_of_password_for_database, authorization_result_in_text ) )
    {
        if ( !writeDownConnectedClientName( login, clientSocket ) )
        {
            authorization_result_in_text += "\n,"+ tr( "but any way - UPS!!! - we got client-named saving mistake." );
            sendToClient( clientSocket, Server_Code::authorization_failed, { { "message", authorization_result_in_text } } );
            return;
        }
        myDatabase_.setActivityStatus( login, 1 );
        sendToClient( clientSocket, Server_Code::authorization_successful, { { "message", authorization_result_in_text } } );
        emit gotNewMesssage( authorization_result_in_text + tr( ". user with login " ) + login + tr( " in system now" ) );
        return;
    }
    emit gotNewMesssage( authorization_result_in_text );
    sendToClient( clientSocket, Server_Code::authorization_failed, { { "message", authorization_result_in_text } } );
}

bool Server::writeDownConnectedClientName( const QString& login, QTcpSocket *ClientSocket )
{
    const int number{ findNumberBySocket( ClientSocket ) };
    if ( number == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't write client into Qmap clients_";
        return false;
    }
    clients_[number].name = login;
    return true;
}

void Server::gotDisconnection()
{
    const int number{ findNumberBySocket( static_cast< QTcpSocket* >( sender() ) ) };
    if ( number == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't remove client from Qmap clients_";
        return;
    }
    clients_[number].socket->destroyed();
    const QString login{ clients_[number].name };
    myDatabase_.setActivityStatus( login, false );
    emit smbDisconnected();
    const int numbers_of_removed_client_in_map{ clients_.remove( number ) };
    qDebug() << __FILE__ << __LINE__  << "numbers_of_removed" << numbers_of_removed_client_in_map << "number:" << number;
}

int Server::sendToClient( QTcpSocket *socket, const Server_Code cur_code )
{
    const QJsonObject obj{ { "code", static_cast< int >( cur_code ) } };
    const QJsonDocument doc( obj );
    const QByteArray jByte( doc.toJson( QJsonDocument::Compact ) );
    return socket->write( jByte );
}

int Server::sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject cur_object )
{
    const QString encrypt_cur_object_inQString{ cryptQJsonObjAndPutItInQString( cur_object, socket ) };
    const QJsonObject obj{ { "code", static_cast< int >( cur_code ) }, { "object", encrypt_cur_object_inQString } };
    const QJsonDocument doc( obj );
    const QByteArray jByte( doc.toJson( QJsonDocument::Compact ) );
    return socket->write( jByte );
}

void Server::sendToClientContactList( QTcpSocket *socket )
{
    if (auto contact_list{ myDatabase_.getActivityStatusAllUsers() }; contact_list!= std::nullopt )
    {
        sendToClient( socket, Server_Code::contacts_list, contact_list.value() );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: not recive contact list to sedt to client";
}

int Server::forwardMessageToRecipient( const QString& recipient, const QJsonObject& messageObj )
{
    const QString author{ messageObj.value( "author" ).toString() };
    const QString timestamp{ messageObj.value( "timestamp" ).toString() };
    const QString message{ messageObj.value( "message" ).toString() };
    const QJsonObject new_message_obj{ { "author", author }, { "recipient", recipient }, { "timestamp", timestamp }, { "message", message } };

    const int size_map{ clients_.size() };
    for ( int i = 0; i < size_map; i++ )
    {
        if ( clients_[i].name == recipient )
        {
            return sendToClient( clients_[i].socket, Server_Code::message_to_recipient, new_message_obj );
        }
    }
    return 0;
}

bool Server::isRecipientconnectedNow( const QString& recipient )
{
    const int size_map{ clients_.size() };
    for ( int i = 0; i < size_map; i++ )
    {
        if ( clients_[i].name == recipient )
            return true;
    }
    return false;
}

int Server::findNumberBySocket( QTcpSocket* current_socket )
{
    const int size_map{ clients_.size() };
    for ( int i = 0; i < size_map; i++ )
    {
        if ( clients_[i].socket == current_socket)
        {
            return i;
        }
    }
    return -1;
}

void Server::calculateAndSetInClientMap( int number_from_client, QTcpSocket *socket )
{
    if ( findNumberBySocket( socket ) == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't calculate session key and set client in map";
        return;
    }
    clients_[ findNumberBySocket( socket )].session_key_object.calculateSessionKey( number_from_client );
}

int Server::getSessionKeyBySocketFromMap( QTcpSocket *socket )
{
    if ( findNumberBySocket( socket ) == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error in method Server::getSessionKeyBySocketFromMap( QTcpSocket *socket )";
        return -1;
    }
    return clients_[findNumberBySocket( socket )].session_key_object.getSessionKey();
}

std::optional< int > Server::getIntermediatNumberBySocketFromMap( QTcpSocket *socket )
{
    if ( findNumberBySocket( socket ) == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error in method Server::getSessionKeyBySocketFromMap( QTcpSocket *socket )";
        return std::nullopt;
    }
    return clients_[findNumberBySocket( socket )].session_key_object.getIntermediateNumberForClient();
}

QByteArray Server::cryptQByteArray ( const QByteArray& jByte, int key )
{
    std::vector< int > key_in_vector{ getVectorDigitsFromNumber( key ) };
    const int vector_size = key_in_vector.size();
    int index{};
    QByteArray jByte_after_crypt;
    foreach( auto item, jByte )
    {
        item = item ^ key_in_vector[index];
        index++;
        if ( index == vector_size )
        {
            index = 0;
        }
        jByte_after_crypt.append( item );
    }
    return jByte_after_crypt;
}

QString Server::cryptQJsonObjAndPutItInQString( const QJsonObject& obj, QTcpSocket *socket )
{
    QJsonDocument doc( obj );
    QByteArray jByte( doc.toJson( QJsonDocument::Compact ) );
    const int key{ getSessionKeyBySocketFromMap( socket ) };
    if ( key == -1 )
        return {};
    const QByteArray crypt_jByte{ cryptQByteArray( jByte, key ) };
    const QString crypt_obj_in_QString( crypt_jByte );
    return crypt_obj_in_QString;
}

QJsonObject Server::decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, QTcpSocket *socket )
{
    QByteArray jByte = encrypt_QString.toUtf8();
    int key = getSessionKeyBySocketFromMap( socket );
    if ( key == -1 )
        return {};
    QByteArray decrypt_jByte = cryptQByteArray( jByte, key );
    QJsonDocument doc = QJsonDocument::fromJson( decrypt_jByte );
    QJsonObject obj = doc.object();
    return obj;
}

std::vector<int> Server::getVectorDigitsFromNumber( int number )
{
    int size{};
    int ostatok{ 1 };
    int temp{ number };
    while ( temp > 0 )
    {
        temp = temp / 10;
        size++;
    }
    temp = number;
    std::vector< int > number_in_vector;
    for ( int i = 0; i < size; ++i)
    {
        ostatok = temp % 10;
        temp = temp / 10;
        number_in_vector.push_back( ostatok );
    }
    return number_in_vector;
}

bool Server::isClientConnect( QTcpSocket *socket )
{
    for (const auto& item: clients_ )
    {
        if ( item.socket == socket )
            return true;
    }
    return false;
}
