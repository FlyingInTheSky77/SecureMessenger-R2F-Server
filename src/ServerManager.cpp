#include "include/ServerManager.h"

ServerManager::ServerManager()
    : QObject()
    , server_( std::make_unique< Server >() )
    , stop_flag_ ( false )
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
        std::unique_lock<std::mutex> lock( mutex_ );
        stop_flag_ = true;
    }
    condition_.notify_all();
    for ( std::thread& worker: threads_ )
    {
        worker.join();
    }
}

QString ServerManager::startServer()
{
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
    qDebug() << __FILE__ << __LINE__ << " dequeueMessage ";

    if ( stop_flag_ && messageQueue_.empty() )
    {
        return std::nullopt;
    }
    auto message = messageQueue_.front();
    messageQueue_.pop();
    return message;
}

void ServerManager::enqueueMessage( QJsonObject obj, QTcpSocket* clientSocket )
{
    {
        std::unique_lock<std::mutex> lock( mutex_ );
        messageQueue_.push( std::make_pair( obj, clientSocket ) );
    }
    qDebug() << __FILE__ << __LINE__ << " enqueueMessage ";
    condition_.notify_one();
}

void ServerManager::threadFunction()
{
    while ( true )
    {
        std::unique_lock< std::mutex > lock( mutex_ );
        qDebug() << __FILE__ << __LINE__ << "befor wait";
        condition_.wait( lock, [ this ] {
            return stop_flag_ || !messageQueue_.empty(); } );

        if (stop_flag_ && messageQueue_.empty()) {
            break; // if the stop flag is set and the queue is empty, we terminate the loop
        }
        lock.unlock();
        qDebug() << __FILE__ << __LINE__ << "after unlock";
        auto messagePackage = dequeueMessage();

        //processMessage( messagePackage ); // TODO implement this in the next steps
     }
}
