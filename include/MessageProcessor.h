#pragma once

#include "Database.h"
#include "messagecode.h"
#include "SessionKey.h"

#include <QTcpSocket>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QList>

#include <optional>
#include <memory>

struct client_struct
{
    QTcpSocket* socket{ nullptr };
    SessionKey session_key_object;
    QString name;
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
    QMap< int, client_struct > getClientsMap();

    void sendClientsServerStoped();

signals:
   void sendToClient_signal( QTcpSocket *socket, const QByteArray jByte );
   void showMessage_signal( QString msg );

private:
    void registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket );
    void authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket );

    void readMessageToServer( const QJsonObject& obj, QTcpSocket* clientSocket );
    void receiveMistake( const QJsonObject& obj, QTcpSocket* clientSocket );

    void readAndForwardMessageToRecipient( const QJsonObject& obj, QTcpSocket* clientSocket );
    int forwardMessageToRecipient( const QString& recipient, const QJsonObject& messageObj );

    void sendToClient( QTcpSocket *socket, const Server_Code cur_code );
    void sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject cur_object );

    void sendToClientContactList( QTcpSocket *socket );
    void sendToAllClientsChangesInClients( const QJsonObject& changes, QTcpSocket *ignore_socket );

    // is user online
    bool isRecipientconnectedNow( const QString& recipient );
    bool isClientConnect( QTcpSocket *socket );
    bool setActivityStatus( const QString& user, bool status );

    // user identification
    int findNumberBySocket( const QTcpSocket* current_socket );
    void calculateAndSetInClientMap( int number_from_client, QTcpSocket *socket );
    bool writeDownConnectedClientName( const QString& login, QTcpSocket *ClientSocket );
    QString findUserBySocket( const QTcpSocket* current_socket );

    std::pair<QString, QString> getCredentials(const QJsonObject& obj, const QTcpSocket *clientSocket);

    // secure session
    std::optional< int > getIntermediatNumberBySocketFromMap( QTcpSocket *socket );
    void secureSessionClientStep( const QJsonObject& obj, QTcpSocket* clientSocket );

    int getSessionKeyBySocketFromMap( const QTcpSocket *socket );
    QByteArray cryptQByteArray ( const QByteArray& jByte, int key );
    QString cryptQJsonObjAndPutItInQString( const QJsonObject& obj, QTcpSocket *socket );
    QJsonObject decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, const QTcpSocket *socket );
    std::vector<int> getVectorDigitsFromNumber( int number );

    Database  myDatabase_;
    QMap< int, client_struct > clients_;
};
