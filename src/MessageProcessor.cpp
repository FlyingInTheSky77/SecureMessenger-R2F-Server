#include <include/MessageProcessor.h>

#include <QJsonObject>
#include <QTcpSocket>

MessageProcessor::MessageProcessor()
    : QObject()
    {}

void MessageProcessor::processIncomingMessages( const QJsonObject& obj, QTcpSocket* clientSocket )
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
            emit showMessage_signal( "we got unknown message from client" );
            sendToClient( clientSocket, Server_Code::mistake, { { "message", "you send to server unknown message-type" } } );
    }
}

void MessageProcessor::processUserDisconnection(QTcpSocket* disconnected_user_socket)
{
    // remove this checking in future:
    const int number{ findNumberBySocket( disconnected_user_socket ) };
    if ( number == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't remove client from Qmap clients_";
        return;
    }
    const QString login = findUserBySocket(disconnected_user_socket);
    setActivityStatus( login, false );

    const int numbers_of_removed_client_in_map{ clients_.remove( number ) };
    qDebug() << __FILE__ << __LINE__  << "numbers_of_removed" << numbers_of_removed_client_in_map << "number:" << number;
}

void MessageProcessor::addNewClientSocket( QTcpSocket* socket )
{
    const int number{ clients_.count() };
    clients_.insert( number, { socket, {},{""} } );
    sendToClient( socket, Server_Code::connection_established );
}

QList< QTcpSocket * > MessageProcessor::getClientsList()
{
    QList< QTcpSocket* > clients_list;
    foreach( auto item, clients_ )
    {
        clients_list << item.socket;
    }
    return clients_list;
}

QMap< int, client_struct > MessageProcessor::getClientsMap()
{
    return clients_;
}

bool MessageProcessor::setActivityStatus( const QString& user, bool status )
{
    if ( myDatabase_.setActivityStatus( user, false ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MessageProcessor::sendToClient( QTcpSocket *socket, const Server_Code cur_code )
{
    const QJsonObject obj{ { "code", static_cast< int >( cur_code ) } };
    const QJsonDocument doc( obj );
    const QByteArray jByte( doc.toJson( QJsonDocument::Compact ) );

    emit sendToClient_signal(socket, jByte);
}

void MessageProcessor::sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject cur_object )
{
    const QString encrypt_cur_object_inQString{ cryptQJsonObjAndPutItInQString( cur_object, socket ) };
    const QJsonObject obj{ { "code", static_cast< int >( cur_code ) }, { "object", encrypt_cur_object_inQString } };
    const QJsonDocument doc( obj );
    const QByteArray jByte( doc.toJson( QJsonDocument::Compact ) );

    emit sendToClient_signal(socket, jByte);
}

void MessageProcessor::sendClientsServerStoped()
{
    QList< QTcpSocket* > clients = getClientsList();
    auto clients_number{ clients.count() };
    for( int i = 0; i < clients_number; i++ )
    {
        sendToClient( clients.at(i), Server_Code::server_stopped,{ { "message", "Connection closed" } } );
    }
}

void MessageProcessor::clearClients()
{
    clients_.clear();
    myDatabase_.setActivityStatusAllUsersToFalse();
}

void MessageProcessor::registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    auto credentials = getCredentials(obj, clientSocket);
    const QString login = credentials.first;
    const QString hashed_password = credentials.second;

    QString registration_result;
    Server_Code code;

    if ( myDatabase_.registration( login, hashed_password, registration_result ) )
    {
        code = Server_Code::registration_successful;
    }
    else
    {
        code = Server_Code::registration_failed;
    }

    emit showMessage_signal( "result of registration with login" + login + registration_result );
    sendToClient( clientSocket, code, { { "message", registration_result } } );
}


void MessageProcessor::authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket )
{
    auto credentials = getCredentials(obj, clientSocket);
    const QString login = credentials.first;
    const QString hashed_password = credentials.second;

    Server_Code code;
    QString authorization_result_in_text;

    if ( myDatabase_.authorization( login, hashed_password, authorization_result_in_text ) )
    {
        if ( writeDownConnectedClientName( login, clientSocket ) )
        {
            code = Server_Code::authorization_successful;
            sendToAllClientsChangesInClients( {{login,true}}, clientSocket );
        }
        else
        {
            code = Server_Code::authorization_failed;
            // in this case, it's necessary to user reconnect to server, not just relogging
            authorization_result_in_text += ". Important: reconnection to server needed.";
        }
    }
    else
    {
        code = Server_Code::authorization_failed;
    }

    emit showMessage_signal( "result of autorization request with login: " + login + " : " + authorization_result_in_text );
    sendToClient( clientSocket, code, { { "message", authorization_result_in_text } } );
}

void MessageProcessor::receiveMistake( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    const QString message_to_server{ decrypt_object.value( "message" ).toString() };

    emit showMessage_signal( "mistake message from client: " + message_to_server );
}

void MessageProcessor::readMessageToServer( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };
    const QString message_to_server{ decrypt_object.value( "message" ).toString() };

    emit showMessage_signal( "received ENCRYPTED message from client to server: " + encrypt_obj_inQString );
    emit showMessage_signal( "message after decryption: "  + message_to_server );
}

void MessageProcessor::readAndForwardMessageToRecipient( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };

    if ( decrypt_object.empty() )
    {
        sendToClient( clientSocket, Server_Code::recipient_offline, { { "message", "client offline for now, send message letter" } } );
        return;
    }

    const QString recipient{ decrypt_object.value( "recipient" ).toString() };
    const QString author{ decrypt_object.value( "author" ).toString() };

    if ( isRecipientconnectedNow( recipient ) )
    {
        if ( !forwardMessageToRecipient( recipient, decrypt_object ) )
        {
            qDebug() << __FILE__ << __LINE__ << "Error on server during forward message to" + recipient + " from" + author;
            emit showMessage_signal( "error forward during forward message to" + recipient + " from" + author);
        }
        return;
    }

    sendToClient( clientSocket, Server_Code::recipient_offline, { { "message", "client offline for now, send message letter" } } );
}

void MessageProcessor::sendToClientContactList( QTcpSocket *socket )
{
    if (auto contact_list{ myDatabase_.getActivityStatusAllUsers() }; contact_list!= std::nullopt )
    {
        sendToClient( socket, Server_Code::contacts_list, contact_list.value() );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: not recive contact list to sedt to client";
}

void MessageProcessor::sendToAllClientsChangesInClients( const QJsonObject& changes, QTcpSocket *ignore_socket )
{
    for (const auto& item: clients_)
    {
        //we don't send the changed status to the client, whose status has changed
        if ( item.socket == ignore_socket )
        {
            continue;
        }

        sendToClient( item.socket, Server_Code::changes_in_contact_list, changes);
    }
}

int MessageProcessor::forwardMessageToRecipient( const QString& recipient, const QJsonObject& messageObj )
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
            sendToClient( clients_[i].socket, Server_Code::message_to_recipient, new_message_obj );
            return 1; // fix it in the next step: another way of errors handling needed
        }
    }
    return 0;
}

bool MessageProcessor::isClientConnect( QTcpSocket *socket )
{
    for (const auto& item: clients_ )
    {
        if ( item.socket == socket )
            return true;
    }
    return false;
}

int MessageProcessor::findNumberBySocket( const QTcpSocket* current_socket )
{
    const int size_map{ clients_.size() };
    for ( int i = 0; i < size_map; i++ )
    {
        if ( clients_[i].socket == current_socket)
            return i;
    }
    return -1;
}

QString MessageProcessor::findUserBySocket( const QTcpSocket* current_socket )
{
    const int size_map{ clients_.size() };
    for ( int i = 0; i < size_map; i++ )
    {
        if ( clients_[i].socket == current_socket)
            return clients_[i].name;
    }
    return{};
}

void MessageProcessor::calculateAndSetInClientMap( int number_from_client, QTcpSocket *socket )
{
    if ( findNumberBySocket( socket ) == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error: Can't calculate session key and set client in map";
        return;
    }
    clients_[ findNumberBySocket( socket )].session_key_object.calculateSessionKey( number_from_client );
}

bool MessageProcessor::writeDownConnectedClientName( const QString& login, QTcpSocket *ClientSocket )
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

std::optional< int > MessageProcessor::getIntermediatNumberBySocketFromMap( QTcpSocket *socket )
{
    if ( findNumberBySocket( socket ) == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error in method Server::getSessionKeyBySocketFromMap( QTcpSocket *socket )";
        return std::nullopt;
    }
    return clients_[findNumberBySocket( socket )].session_key_object.getIntermediateNumberForClient();
}

void MessageProcessor::secureSessionClientStep( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    int intermediate_number_from_client = obj.value( "first_step" ).toInt();
    calculateAndSetInClientMap( intermediate_number_from_client, clientSocket );

    if ( auto m_intermediate_number = getIntermediatNumberBySocketFromMap( clientSocket ); m_intermediate_number != std::nullopt )
    {
        const QJsonObject cur_object{ { "code", static_cast< int >( Server_Code::secure_session_server_step) }, { "second_step", m_intermediate_number.value() } };
        const QJsonDocument doc_encrypt( cur_object );
        const QByteArray jByte_encrypt( doc_encrypt.toJson( QJsonDocument::Compact ) );

        emit sendToClient_signal( clientSocket, jByte_encrypt );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: getIntermediatNumberBySocketFromMap";
}

int MessageProcessor::getSessionKeyBySocketFromMap( const QTcpSocket *socket )
{
    if ( findNumberBySocket( socket ) == -1 )
    {
        qDebug() << __FILE__ << __LINE__ << "Error in method Server::getSessionKeyBySocketFromMap( QTcpSocket *socket )";
        return -1;
    }
    return clients_[findNumberBySocket( socket )].session_key_object.getSessionKey();
}

QByteArray MessageProcessor::cryptQByteArray ( const QByteArray& jByte, int key )
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
            index = 0;
        jByte_after_crypt.append( item );
    }
    return jByte_after_crypt;
}

QString MessageProcessor::cryptQJsonObjAndPutItInQString( const QJsonObject& obj, QTcpSocket *socket )
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

QJsonObject MessageProcessor::decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, const QTcpSocket *socket )
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

std::vector<int>  MessageProcessor::getVectorDigitsFromNumber( int number )
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

bool MessageProcessor::isRecipientconnectedNow( const QString& recipient )
{
    const int size_map{ clients_.size() };
    for ( int i = 0; i < size_map; i++ )
    {
        if ( clients_[i].name == recipient )
            return true;
    }
    return false;
}

std::pair<QString, QString> MessageProcessor::getCredentials(const QJsonObject& obj, const QTcpSocket *clientSocket)
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };

    const QString login{ decrypt_object.value( "login" ).toString() };

    const QString password{ decrypt_object.value( "password" ).toString() };
    const QByteArray password_in_QByteArray{ password.toUtf8() };
    const QByteArray hashed_password_in_QByteArray{ QCryptographicHash::hash( password_in_QByteArray, QCryptographicHash::Sha1 ) };
    const std::string hashed_password_std{ hashed_password_in_QByteArray.toStdString() };
    const QString hashed_password{ QString::fromUtf8( hashed_password_std.c_str() ) };

    return {login, hashed_password};
}