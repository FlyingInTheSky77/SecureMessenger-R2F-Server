#include "BackEnd.h"

#include <QString>

BackEnd::BackEnd( QObject *parent )
    : QObject( parent )
    , serverManager_( std::make_unique< ServerManager >() )
{
    serverManager_->startServer();
    qDebug() << "Server started";

    connect( serverManager_.get(), &ServerManager::gotNewMesssage_signal, this, &BackEnd::gotNewMesssage );
    connect( serverManager_.get(), &ServerManager::smbConnected_signal, this, &BackEnd::smbConnectedToServer );
    connect( serverManager_.get(), &ServerManager::smbDisconnected_signal, this, &BackEnd::smbDisconnectedFromServer );
}

BackEnd::~BackEnd()
{
    disconnect( serverManager_.get(), &ServerManager::gotNewMesssage_signal, this, &BackEnd::gotNewMesssage );
    disconnect( serverManager_.get(), &ServerManager::smbConnected_signal, this, &BackEnd::smbConnectedToServer );
    disconnect( serverManager_.get(), &ServerManager::smbDisconnected_signal, this, &BackEnd::smbDisconnectedFromServer );

    serverManager_->stopServer();
    qDebug() << "Server stoped";
}

void BackEnd::smbConnectedToServer()
{
    qDebug() << "somebody connected";
    qDebug() << serverManager_->showServerStatus();
}

void BackEnd::smbDisconnectedFromServer()
{    
    qDebug() << "somebody connected";
    qDebug() << serverManager_->showServerStatus();
}

void BackEnd::gotNewMesssage( QString message )
{
    qDebug() << "new message to server: " << message;
}
