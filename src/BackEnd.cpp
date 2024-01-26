#include "include/server.h"
#include "include/BackEnd.h"

BackEnd::BackEnd( QObject *parent )
    : QObject{ parent }
    , server_{ std::make_unique< Server >() }
{
    connect( server_.get(), &Server::gotNewMesssage_signal, this, &BackEnd::gotNewMesssage );
    connect( server_.get(), &Server::smbConnected_signal, this, &BackEnd::smbConnectedToServer );
    connect( server_.get(), &Server::smbDisconnected_signal, this, &BackEnd::smbDisconnectedFromServer );
}

BackEnd::~BackEnd()
{
    disconnect( server_.get(), &Server::gotNewMesssage_signal, this, &BackEnd::gotNewMesssage );
    disconnect( server_.get(), &Server::smbConnected_signal, this, &BackEnd::smbConnectedToServer );
    disconnect( server_.get(), &Server::smbDisconnected_signal, this, &BackEnd::smbDisconnectedFromServer );
}

QString BackEnd::startClicked()
{
    return server_->start();
}

QString BackEnd::stopClicked()
{
    return server_->stop();
}

QString BackEnd::testConnectionClicked()
{
    return server_->showServerStatus();
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
