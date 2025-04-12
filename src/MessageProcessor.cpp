#include "MessageProcessor.h"

#include <QCryptographicHash>
#include <QJsonParseError>

MessageProcessor::MessageProcessor(QObject *parent)
    : QObject( parent )
{}

MessageProcessor::~MessageProcessor()
{
    clearClients();
}

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
            processMessageToAnotherClient( obj, clientSocket );
            break;
        case Client_Code::secure_session_client_step:
            secureSessionServerStep( obj, clientSocket );
            break;
        case Client_Code::mistake:
            processReceivedMistake( obj, clientSocket );
            break;
        default:
            emit showMessage_signal( "we got unknown message from client" );
            sendToClient( clientSocket, Server_Code::mistake, { { "message", "you send to server unknown message-type" } } );
    }
}

void MessageProcessor::processUserDisconnection( QTcpSocket* disconnected_user_socket )
{
    const Server_Code code = Server_Code::user_offline;
    auto disconnectedUserLogin = connected_client_manager_.getUserLogin( disconnected_user_socket );
    const QJsonObject userInfo{{"login", disconnectedUserLogin},{"activityStatus","offline"}};
    sendContactUpdateToClients( code, userInfo , disconnected_user_socket );
    setClientActivityStatus( disconnectedUserLogin, "offline" );
    connected_client_manager_.removeUser( disconnected_user_socket );
}

void MessageProcessor::addNewClientSocket( QTcpSocket* socket )
{
    connected_client_manager_.addNewClientSocket( socket );

    sendToClient( socket, Server_Code::connection_established );
}

QList< QTcpSocket * > MessageProcessor::getClientSocketsList()
{
    return connected_client_manager_.getClientSocketsList();
}

int MessageProcessor::getNumberConnectedClients()
{
    return connected_client_manager_.getNumberConnectedClients();
}

bool MessageProcessor::setClientActivityStatus( const QString& login, const QString& status )
{
    if ( user_credentials_database_.setActivityStatus( login, status ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MessageProcessor::sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject received_object )
{
    QJsonObject sent_object{};
    if ( !received_object.empty() )
    {
        const QString encrypt_received_object_inQString{ cryptQJsonObjAndPutItInQString( received_object, socket ) };
        sent_object = { { "code", static_cast< int >( cur_code ) }, { "object", encrypt_received_object_inQString } };
    }
    else
    {
        sent_object = { { "code", static_cast< int >( cur_code ) } };
    }
    const QJsonDocument doc( sent_object );
    const QByteArray jByte( doc.toJson( QJsonDocument::Compact ) );

    emit sendToClient_signal( socket, jByte );
}

void MessageProcessor::sendClientsServerStoped()
{
    QList< QTcpSocket* > list_of_clients_sockets = connected_client_manager_.getClientSocketsList();
    for ( auto socket: list_of_clients_sockets )
    {
        sendToClient( socket, Server_Code::server_stopped,{ { "message", "Connection closed" } } );
    }
}

void MessageProcessor::clearClients()
{
    connected_client_manager_.clearClients();

    user_credentials_database_.setActivityStatusAllUsersToFalse();
}

void MessageProcessor::registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    auto credentials = getClientCredentials( obj, clientSocket );
    const QString login = credentials.first;
    const QString hashed_password = credentials.second;

    Server_Code code;
    Identification_request registration_result = user_credentials_database_.registration( login, hashed_password );

    if ( registration_result.is_request_granted )
    {

        if ( saveUserName( login, clientSocket ) )
        {
            code = Server_Code::registration_successful;
            const QJsonObject userInfo{{"login", login},{"activityStatus","online"}};
            qDebug() << __FILE__ << __LINE__ << "registration_successful";

            sendContactUpdateToClients( Server_Code::registred_new_user, userInfo, clientSocket );
        }
        else
        {
            code = Server_Code::registration_failed;
            // in this case, it's necessary to user reconnect to server, not just relogging
            registration_result.message +=  ". Important: reconnection to server needed.";
            qDebug() << __FILE__ << __LINE__ << "Important: reconnection to server needed";
        }
    }
    else
    {
        code = Server_Code::registration_failed;
        qDebug() << __FILE__ << __LINE__ << "registration_failed";
    }

    emit showMessage_signal( "result of registration request with login \"" + login +"\" : " +  registration_result.message );
    sendToClient( clientSocket, code, { { "message", registration_result.message } } );
    qDebug() << __FILE__ << __LINE__ << "result of registration request with login \"" + login +"\" : " +  registration_result.message;
}

void MessageProcessor::authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket )
{
    auto credentials = getClientCredentials( obj, clientSocket );
    const QString login = credentials.first;
    const QString hashed_password = credentials.second;

    Server_Code code;
    Identification_request authorization_result = user_credentials_database_.authorization( login, hashed_password );

    if ( authorization_result.is_request_granted )
    {
        if ( saveUserName( login, clientSocket ) )
        {
            code = Server_Code::authorization_successful;
            const QJsonObject userInfo{ { "login", login },{ "activityStatus","online" } };

            sendContactUpdateToClients( Server_Code::user_online, userInfo, clientSocket );
        }
        else
        {
            code = Server_Code::technical_errors_on_server;
            // in this case, it's necessary to user reconnect to server, not just relogging
            authorization_result.message += ". Important: reconnection to server needed.";
            qDebug() << "Important: reconnection to server needed";
        }
    }
    else
    {
        code = Server_Code::authorization_failed;
    }

    emit showMessage_signal( "result of autorization request with login \"" + login + "\" : " + authorization_result.message );
    sendToClient( clientSocket, code, { { "message", authorization_result.message } } );
}

void MessageProcessor::processReceivedMistake( const QJsonObject& obj, QTcpSocket* clientSocket )
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

void MessageProcessor::processMessageToAnotherClient( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };

    if ( decrypt_object.empty() )
    {
        qDebug() << __FILE__ << __LINE__ << "The sender disconnected from the server very quickly";
        // this will be useful in case of multi-threaded processing of incoming messages from clients
    }
    else
    {
        const QString recipient{ decrypt_object.value( "recipient" ).toString() };
        QTcpSocket* recipient_socket = getSocketIfUserIsAuthorized( recipient );

        if (user_credentials_database_.checkIfUserWithLoginExists ( recipient )) {
            qDebug() << "User with this login \"" << recipient << "\" doesn't exist";
            sendToClient( clientSocket, Server_Code::recipient_not_registered, { { "message", "Message wasn't forwarded to recipient, because User with this login + recipient + doesn't exist" } } );
        }

        if ( recipient_socket )
        {
            forwardMessageToRecipient( recipient, recipient_socket, decrypt_object );
        }
        else
        {
            sendToClient( clientSocket, Server_Code::recipient_offline, { { "message", "Message wasn't forwarded to recipient, because he is offline. Send message letter" } } );
        }
    }
}

void MessageProcessor::sendToClientContactList( QTcpSocket *socket )
{
    if (auto contact_list{ user_credentials_database_.getActivityStatusAllUsers() }; contact_list!= std::nullopt )
    {
        sendToClient( socket, Server_Code::contacts_list, contact_list.value() );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: contact list not received";
}

void MessageProcessor::sendContactUpdateToClients( const Server_Code server_code, const QJsonObject& changes, const QTcpSocket *ignore_socket )
{
    QList < QTcpSocket* > list_of_sockets = connected_client_manager_.getClientSocketsList();
    for ( auto& current_socket : list_of_sockets )
    {
        if ( current_socket != ignore_socket )
        {

            sendToClient( current_socket, server_code, changes );
        }
        else
        {
            //we don't send the changed status to the client, whose status has changed
        }
    }
}

void MessageProcessor::forwardMessageToRecipient( const QString& recipient, QTcpSocket* recipient_socket, const QJsonObject& messageObj )
{
    const QString author{ messageObj.value( "author" ).toString() };
    const QString timestamp{ messageObj.value( "timestamp" ).toString() };
    const QString message{ messageObj.value( "message" ).toString() };
    const QJsonObject message_json { { "author", author }, { "recipient", recipient }, { "timestamp", timestamp }, { "message", message } };

    sendToClient( recipient_socket, Server_Code::message_to_recipient, message_json );
}

void MessageProcessor::calculateAndSaveClientSessionKey( int number_from_client, QTcpSocket *socket )
{
    connected_client_manager_.calculateAndSaveClientSessionKey( number_from_client, socket );
}

bool MessageProcessor::saveUserName( const QString& login, QTcpSocket *ClientSocket )
{
    return connected_client_manager_.saveUserName( login, ClientSocket );
}

std::optional< int > MessageProcessor::getClientIntermediateNumber( QTcpSocket *socket )
{
    return connected_client_manager_.getClientIntermediateNumber( socket );
}

void MessageProcessor::secureSessionServerStep( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    int intermediate_number_from_client = obj.value( "first_step" ).toInt();
    calculateAndSaveClientSessionKey( intermediate_number_from_client, clientSocket );

    if ( auto intermediate_number = getClientIntermediateNumber( clientSocket ); intermediate_number != std::nullopt )
    {
        const QJsonObject cur_object{ { "code", static_cast< int >( Server_Code::secure_session_server_step) }, { "second_step", intermediate_number.value() } };
        const QJsonDocument doc_encrypt( cur_object );
        const QByteArray jByte_encrypt( doc_encrypt.toJson( QJsonDocument::Compact ) );

        emit sendToClient_signal( clientSocket, jByte_encrypt );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: getIntermediatNumberBySocketFromMap";
}

int MessageProcessor::getUserSessionKey( QTcpSocket *socket )
{
    return connected_client_manager_.getUserSessionKey( socket );
}

QByteArray MessageProcessor::cryptQByteArray ( const QByteArray& jByte, int key )
{
    std::vector< int > key_in_vector{ convertNumberIntoVectorOfItsDigits( key ) };
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
    const QString user_login = connected_client_manager_.getUserLogin( socket );
    qDebug() << __FILE__ << __LINE__ << "User login :" << user_login;

    const int key{ getUserSessionKey( socket ) };

    if ( key == -1 )
        return {};
    const QByteArray crypt_jByte{ cryptQByteArray( jByte, key ) };
    const QString crypt_obj_in_QString( crypt_jByte );
    return crypt_obj_in_QString;
}

QJsonObject MessageProcessor::decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, QTcpSocket *socket )
{
    QByteArray jByte = encrypt_QString.toUtf8();
    const int key = getUserSessionKey( socket );
    if ( key == -1 )
        return {};
    QByteArray decrypt_jByte = cryptQByteArray( jByte, key );
    QJsonDocument doc = QJsonDocument::fromJson( decrypt_jByte );
    QJsonObject obj = doc.object();
    return obj;
}

std::vector<int>  MessageProcessor::convertNumberIntoVectorOfItsDigits( int number )
{
    int size{};
    int remainder{ 1 };
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
        remainder = temp % 10;
        temp = temp / 10;
        number_in_vector.push_back( remainder );
    }
    return number_in_vector;
}

QTcpSocket* MessageProcessor::getSocketIfUserIsAuthorized( const QString& login )
{
    return connected_client_manager_.getSocketIfUserIsAuthorized( login );
}

std::pair<QString, QString> MessageProcessor::getClientCredentials(const QJsonObject& obj, QTcpSocket *clientSocket)
{
    const QString encrypt_obj_inQString{ obj.value( "object" ).toString() };
    const QJsonObject decrypt_object{ decryptQJsonObjFromEncryptQString( encrypt_obj_inQString, clientSocket ) };

    const QString login{ decrypt_object.value( "login" ).toString() };

    const QString password{ decrypt_object.value( "password" ).toString() };
    const QByteArray password_in_QByteArray{ password.toUtf8() };
    const QByteArray hashed_password_in_QByteArray{ QCryptographicHash::hash( password_in_QByteArray, QCryptographicHash::Sha1 ) };
    const std::string hashed_password_std{ hashed_password_in_QByteArray.toStdString() };
    const QString hashed_password{ QString::fromUtf8( hashed_password_std.c_str() ) };

    return { login, hashed_password };
}
