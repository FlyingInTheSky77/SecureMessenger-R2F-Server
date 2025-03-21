#include "ServerManager.h"

ServerManager::ServerManager(QObject *parent)
    : QObject( parent )
    , server_( std::make_unique< Server >() )
    , done_ ( false )
{
    auto numThreads = std::thread::hardware_concurrency();
    threads_.reserve( numThreads );
    for( size_t i = 0; i < numThreads; ++i )
    {
        qDebug() << __FILE__ << __LINE__ << " numThreads = " << numThreads;
        threads_.emplace_back( &ServerManager::threadFunction, this );
    }

    connect( server_.get(), &Server::gotNewMesssage_signal, this, &ServerManager::gotNewMesssage_signal );
    connect( server_.get(), &Server::smbConnected_signal, this, &ServerManager::smbConnected_signal );
    connect( server_.get(), &Server::smbDisconnected_signal, this, &ServerManager::smbDisconnected_signal );

    connect( server_.get(), &Server::recivedMessageFromClient_signal, this, &ServerManager::enqueueMessage );
}

ServerManager::~ServerManager()
{
    disconnect( server_.get(), &Server::gotNewMesssage_signal, this, &ServerManager::gotNewMesssage_signal );
    disconnect( server_.get(), &Server::smbConnected_signal, this, &ServerManager::smbConnected_signal );
    disconnect( server_.get(), &Server::smbDisconnected_signal, this, &ServerManager::smbDisconnected_signal );

    disconnect( server_.get(), &Server::recivedMessageFromClient_signal, this, &ServerManager::enqueueMessage );

    {
        std::lock_guard< std::mutex > lock( mutex_ );
        done_ = true;
    }
    condition_.notify_all();
    for ( std::thread& worker: threads_ )
    {
        worker.join();
    }
}

QString ServerManager::startServer()
{
    qDebug() << "Server started";
    return server_->start();
}

QString ServerManager::stopServer()
{
    return server_->stop();
}

QString ServerManager::showServerStatus()
{
    return server_->showServerStatus();
}

std::optional < std::pair<QJsonObject, QTcpSocket *> > ServerManager::dequeueMessage()
{
    std::unique_lock< std::mutex > lock( mutex_ );
    while ( messageQueue_.empty() && !done_ )
    {
        condition_.wait( lock );
    }
    if ( messageQueue_.empty() )
    {
        return std::nullopt;
    }

    auto message = messageQueue_.front();
    messageQueue_.pop();
    return message;
}

void ServerManager::enqueueMessage( QJsonObject obj, QTcpSocket* clientSocket )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    messageQueue_.push( std::make_pair( obj, clientSocket ) );
    condition_.notify_one();
}

void ServerManager::done()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    done_ = true;
    condition_.notify_all();
}

void ServerManager::threadFunction()
{
    while ( auto message_package_opt = dequeueMessage() )
    {
        auto message_package  = *message_package_opt;
        //processMessage( message_package ); // TODO implement this in the next steps
    }
}
