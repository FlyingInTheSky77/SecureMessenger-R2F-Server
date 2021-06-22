#pragma once
#include "stdafx.h"
#include "Database.h"
#include "SessionKey.h"
#include "messagecode.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QList>
#include <QTime>
#include <QJsonObject>
#include <QCryptographicHash>

#include <optional>

struct client_struct
{
    QTcpSocket* socket{ nullptr };
    SessionKey session_key_object;
    QString name;
};

class Server: public QObject
{
    Q_OBJECT

public:
    Server();
    std::unique_ptr< QTcpServer > tcpServer_;
    QList< QTcpSocket* > getClients();
    int sendToClient( QTcpSocket *socket, const Server_Code cur_code );
    int sendToClient( QTcpSocket *socket, const Server_Code cur_code, QJsonObject cur_object );
    void disconnectSockets();

public slots:
    virtual void newConnection();
    void readClient();
    void gotDisconnection();
    void sendToClientContactList( QTcpSocket *socket );

signals:
    void gotNewMesssage( QString msg );
    void smbDisconnected();

private:    
    QMap< int, client_struct > clients_;
    Database  myDatabase_;

    void readAndForwardMessageToRecipient( const QJsonObject& obj, QTcpSocket* clientSocket );
    int forwardMessageToRecipient( const QString& recipient, const QJsonObject& messageObj );
    int findNumberBySocket( QTcpSocket* current_socket );
    bool isRecipientconnectedNow( const QString& recipient );
    void registrationRequest( const QJsonObject& obj, QTcpSocket* clientSocket );
    void authorizationRequest( const QJsonObject& obj, QTcpSocket *clientSocket );
    bool writeDownConnectedClientName( const QString& login, QTcpSocket *ClientSocket );
    void readMessageToServer( const QJsonObject& obj, QTcpSocket* clientSocket );
    void letsSeeWhatWeGotFromClient( const QJsonObject& obj, QTcpSocket* clientSocket );
    bool isClientConnect( QTcpSocket *socket );
    void sendToAllClientsChangesInClients( const QJsonObject& changes, QTcpSocket *ignore_socket );

    void calculateAndSetInClientMap( int intermediat_number_from_client, QTcpSocket *socket );
    int getSessionKeyBySocketFromMap( QTcpSocket *socket );
    std::optional< int > getIntermediatNumberBySocketFromMap( QTcpSocket *socket );
    void secureSessionClientStep( const QJsonObject& obj, QTcpSocket* clientSocket );
    void receiveMistake( const QJsonObject& obj, QTcpSocket* clientSocket );

    QByteArray cryptQByteArray( const QByteArray& jByte, int key );
    QString cryptQJsonObjAndPutItInQString( const QJsonObject& obj, QTcpSocket *socket );
    QJsonObject decryptQJsonObjFromEncryptQString( const QString& encrypt_QString, QTcpSocket *socket );
    std::vector< int > getVectorDigitsFromNumber( int number );
};
