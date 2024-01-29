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

void MessageProcessor::addNewClientSocket( QTcpSocket* socket )
{
    clients_.insert( socket, { {},{""} } );

    sendToClient( socket, Server_Code::connection_established );
}

QList< QTcpSocket * > MessageProcessor::getClientSocketsList()
{
    return clients_.keys();
}

int MessageProcessor::getNumberConnectedClients()
{
    return clients_.count();
}

bool MessageProcessor::setActivityStatus( const QString& user, bool status )
{
    if ( myDatabase_.setActivityStatus( user, status ) )
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
    QList < QTcpSocket* > list_of_clients_sockets = clients_.keys();
    for (auto socket: list_of_clients_sockets)
    {
        sendToClient( socket, Server_Code::server_stopped,{ { "message", "Connection closed" } } );
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

    Server_Code code;
    Identification_request registration_result = myDatabase_.registration( login, hashed_password );

    if ( registration_result.is_request_granted )
    {

        if ( saveUserName( login, clientSocket ) )
        {
            code = Server_Code::registration_successful;
            QJsonObject login_and_status_one_user{{"login", login},{"activityStatus","true"}};
            qDebug() << __FILE__ << __LINE__ << "registration_successful";

            sendToAllClientsChangesInClients( login_and_status_one_user, clientSocket );
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
    auto credentials = getCredentials(obj, clientSocket);
    const QString login = credentials.first;
    const QString hashed_password = credentials.second;

    Server_Code code;
    Identification_request authorization_result = myDatabase_.authorization( login, hashed_password );

    if ( authorization_result.is_request_granted )
    {
        if ( saveUserName( login, clientSocket ) )
        {
            code = Server_Code::authorization_successful;
            QJsonObject login_and_status_one_user{{"login", login},{"activityStatus","true"}};

            sendToAllClientsChangesInClients( login_and_status_one_user, clientSocket );
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

    QTcpSocket* recipient_socket = getSocketIfRecipientConnected( recipient );
    if ( recipient_socket )
    {
        forwardMessageToRecipient( recipient, recipient_socket, decrypt_object );
    }
    else
    {
        sendToClient( clientSocket, Server_Code::recipient_offline, { { "message", "client offline for now, send message letter" } } );
    }
}

void MessageProcessor::sendToClientContactList( QTcpSocket *socket )
{
    if (auto contact_list{ myDatabase_.getActivityStatusAllUsers() }; contact_list!= std::nullopt )
    {
        sendToClient( socket, Server_Code::contacts_list, contact_list.value() );
        return;
    }
    qDebug() << __FILE__ << __LINE__ << "Error: contact list not received";
}

void MessageProcessor::sendToAllClientsChangesInClients( const QJsonObject& changes, const QTcpSocket *ignore_socket )
{
    for ( QMap<QTcpSocket*, client_struct>::iterator it = clients_.begin(); it != clients_.end(); ++it )
    {
        QTcpSocket* current_socket = it.key();

        //we don't send the changed status to the client, whose status has changed
        if ( current_socket == ignore_socket )
        {
            continue;
        }
        sendToClient( current_socket, Server_Code::changes_in_contact_list, changes);
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

void MessageProcessor::calculateAndSetInClientMap( int number_from_client, QTcpSocket *socket )
{
    if ( clients_.contains( socket ) )
    {
        clients_[ socket ].session_key_object.calculateSessionKey( number_from_client );
    }
}

bool MessageProcessor::saveUserName( const QString& login, QTcpSocket *ClientSocket )
{
    if ( clients_.contains( ClientSocket ) )
    {
        clients_[ClientSocket].login = login;
        return true;
    }
    else
    {
        return false;
    }
}

std::optional< int > MessageProcessor::getIntermediatNumberBySocketFromMap( QTcpSocket *socket )
{
    if ( clients_.contains( socket ) )
    {
        return clients_[socket].session_key_object.getIntermediateNumberForClient();
    }
    else
    {
        return std::nullopt;
    }
}

void MessageProcessor::secureSessionClientStep( const QJsonObject& obj, QTcpSocket* clientSocket )
{
    int intermediate_number_from_client = obj.value( "first_step" ).toInt();
    calculateAndSetInClientMap( intermediate_number_from_client, clientSocket );

    if ( auto intermediate_number = getIntermediatNumberBySocketFromMap( clientSocket ); intermediate_number != std::nullopt )
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
    qDebug() << __FILE__ << __LINE__ << "clients[socket].login :" <<clients_[socket].login ;
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

QTcpSocket* MessageProcessor::getSocketIfRecipientConnected( const QString& recipient )
{
    QTcpSocket *recipient_socket = nullptr;

    for ( QMap<QTcpSocket*, client_struct>::iterator it = clients_.begin(); it != clients_.end(); ++it )
    {
        if (it.value().login == recipient)
        {
            recipient_socket = it.key();
            break;
        }
    }
    return recipient_socket;
}

std::pair<QString, QString> MessageProcessor::getCredentials(const QJsonObject& obj, QTcpSocket *clientSocket)
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
