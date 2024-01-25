#pragma once

#include "Database.h"
#include "messagecode.h"
#include "SessionKey.h"

#include <QTcpSocket>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QList>

#include <optional>

struct client_struct
{
    SessionKey session_key_object;
    QString login;
};

class MessageProcessor: public QObject
{

    Q_OBJECT

public:
    MessageProcessor();
    void processIncomingMessages(const QJsonObject& obj, QTcpSocket* clientSocket);
    void processUserDisconnection(QTcpSocket* disconnected_user_socket);

    void clearClients();
    void addNewClientSocket(QTcpSocket* socket);
    QList< QTcpSocket * > getClientsList();
    QMap< QTcpSocket*, client_struct > getClientsMap();

    void sendClientsServerStoped();

signals:
   void sendToClient_signal( QTcpSocket *socket, const QByteArray jByte );
   void showMessage_signal( QString msg );

private:
    // user identification
    void registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket );
    void authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket );

    void readMessageToServer( const QJsonObject& obj, QTcpSocket* clientSocket );
    void receiveMistake( const QJsonObject& obj, QTcpSocket* clientSocket );

    void readAndForwardMessageToRecipient( const QJsonObject& obj, QTcpSocket* clientSocket );
    void forwardMessageToRecipient( const QString& recipient, QTcpSocket* recipient_socket, const QJsonObject& messageObj );

    void sendToClient( QTcpSocket *socket, const Server_Code cur_code );
    void sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject cur_object );

    void sendToClientContactList( QTcpSocket *socket );
    void sendToAllClientsChangesInClients( const QJsonObject& changes, const QTcpSocket *ignore_socket );

    // is user online
    QTcpSocket* getSocketIfRecipientConnected( const QString& recipient );
    bool setActivityStatus( const QString& user, bool status );

    void calculateAndSetInClientMap( int number_from_client, QTcpSocket *socket );
    bool saveUserName( const QString& login, QTcpSocket *ClientSocket );

    std::pair<QString, QString> getCredentials(const QJsonObject& obj, QTcpSocket *clientSocket);

    // secure session
    std::optional< int > getIntermediatNumberBySocketFromMap( QTcpSocket *socket );
    void secureSessionClientStep( const QJsonObject& obj, QTcpSocket* clientSocket );

    int getUserSessionKey( QTcpSocket *socket );
    QByteArray cryptQByteArray ( const QByteArray& jByte, int key );
    QString cryptQJsonObjAndPutItInQString( const QJsonObject& obj, QTcpSocket *socket );
    QJsonObject decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, QTcpSocket *socket );
    std::vector<int> getVectorDigitsFromNumber( int number );

    QMap< QTcpSocket*, client_struct > clients_;
    Database  myDatabase_;
};
