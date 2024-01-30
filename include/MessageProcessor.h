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
    QList< QTcpSocket * > getClientSocketsList();
    int getNumberConnectedClients();

    void sendClientsServerStoped();

signals:
   void sendToClient_signal( QTcpSocket *socket, const QByteArray jByte );
   void showMessage_signal( QString msg );

private:
    // user identification
    void registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket );
    void authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket );

    void readMessageToServer( const QJsonObject& obj, QTcpSocket* clientSocket );
    void processReceivedMistake( const QJsonObject& obj, QTcpSocket* clientSocket );

    void processMessageToAnotherClient( const QJsonObject& obj, QTcpSocket* clientSocket );
    void forwardMessageToRecipient( const QString& recipient, QTcpSocket* recipient_socket, const QJsonObject& messageObj );

    void sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject received_object = {});

    void sendToClientContactList( QTcpSocket *socket );
    void sendClientActivityUpdateToAllClients( const QJsonObject& changes, const QTcpSocket *ignored_socket );

    // is user online
    QTcpSocket* getSocketIfUserIsAuthorized( const QString& login );
    bool setClientActivityStatus( const QString& login, bool status );

    void calculateAndSaveClientSessionKey( int number_from_client, QTcpSocket *socket );
    bool saveUserName( const QString& login, QTcpSocket *ClientSocket );

    std::pair<QString, QString> getClientCredentials(const QJsonObject& obj, QTcpSocket *clientSocket);

    // secure session
    std::optional< int > getClientIntermediateNumber( QTcpSocket *socket );
    void secureSessionServerStep( const QJsonObject& obj, QTcpSocket* clientSocket );

    int getUserSessionKey( QTcpSocket *socket );
    QByteArray cryptQByteArray ( const QByteArray& jByte, int key );
    QString cryptQJsonObjAndPutItInQString( const QJsonObject& obj, QTcpSocket *socket );
    QJsonObject decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, QTcpSocket *socket );
    std::vector<int> convertNumberIntoVectorOfItsDigits( int number );

    QMap< QTcpSocket*, client_struct > clients_;
    Database  myDatabase_;
};
