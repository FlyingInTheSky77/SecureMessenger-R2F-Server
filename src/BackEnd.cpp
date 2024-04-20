#include "../include/ServerManager.h"
#include "../include/BackEnd.h"

BackEnd::BackEnd( QObject *parent )
    : QObject{ parent }
    , serverManager_{ std::make_unique< ServerManager >() }
{
    connect( serverManager_.get(), &ServerManager::gotNewMesssage_signal, this, &BackEnd::gotNewMesssage );
    connect( serverManager_.get(), &ServerManager::smbConnected_signal, this, &BackEnd::smbConnectedToServer );
    connect( serverManager_.get(), &ServerManager::smbDisconnected_signal, this, &BackEnd::smbDisconnectedFromServer );
}

BackEnd::~BackEnd()
{
    disconnect( serverManager_.get(), &ServerManager::gotNewMesssage_signal, this, &BackEnd::gotNewMesssage );
    disconnect( serverManager_.get(), &ServerManager::smbConnected_signal, this, &BackEnd::smbConnectedToServer );
    disconnect( serverManager_.get(), &ServerManager::smbDisconnected_signal, this, &BackEnd::smbDisconnectedFromServer );
}

QString BackEnd::startClicked()
{
    return serverManager_->startServer();
}

QString BackEnd::stopClicked()
{
    return serverManager_->stopServer();
}

QString BackEnd::showServerStatusClicked()
{
    return serverManager_->showServerStatus();
}

void BackEnd::smbConnectedToServer()
{
    emit smbConnected_signal();
}

void BackEnd::smbDisconnectedFromServer()
{    
    emit smbDisconnected_signal();
}

void BackEnd::gotNewMesssage( QString message )
{
    emit newMessage_signal( message );
}
