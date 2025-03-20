#include "TcpDumpManager.h"

#include <QDebug>

TcpDumpManager::TcpDumpManager(QObject *parent) : QObject(parent)
{
}

void TcpDumpManager::startTcpDump()
{
    if ( !tcpDumpProcess_ptr_ )
    {
        tcpDumpProcess_ptr_ = std::make_unique< QProcess >();
        qDebug() << "tcpDumpProcess is starting ";
    }
    else
    {
        qDebug() << "tcpDumpProcess already started";
    }

    QString command = QString("sudo tcpdump -i lo port 5555 -A -l");
    tcpDumpProcess_ptr_->start( command );

    qDebug() << "Current process state:" << tcpDumpProcess_ptr_->state();

    if (!tcpDumpProcess_ptr_ -> waitForStarted()) {
        // Handle process start error
        qDebug() << "Handle process start error";
    }
    else {
        qDebug() << "Handle process start OK";
    }

     qDebug() << "Current process state:" << tcpDumpProcess_ptr_->state();

    connect(tcpDumpProcess_ptr_.get(), &QProcess::readyReadStandardOutput, this, &TcpDumpManager::readTcpDumpOutput);
    connect(tcpDumpProcess_ptr_.get(), &QProcess::errorOccurred, this, &TcpDumpManager::processError);
    connect(tcpDumpProcess_ptr_.get(), &QProcess::started, this, &TcpDumpManager::onProcessStarted);
    connect(tcpDumpProcess_ptr_.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TcpDumpManager::onProcessFinished);

    connect(tcpDumpProcess_ptr_.get(), static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::errorOccurred),
            [=](QProcess::ProcessError error){
                qDebug() << "Error occurred:" << error;
            });

    connect(tcpDumpProcess_ptr_.get(), &QProcess::errorOccurred, this, [](QProcess::ProcessError error) {
        qDebug() << "Process error occurred:" << error;
    });

    connect(tcpDumpProcess_ptr_.get(), &QProcess::stateChanged, this, [](QProcess::ProcessState newState) {
        qDebug() << "Process state changed to:" << newState;
    });

    if ( !packetAnalyzer_ptr_ )
    {
        packetAnalyzer_ptr_ = std::make_unique< PacketAnalyzer >();
    }
}

void TcpDumpManager::readTcpDumpOutput()
{
    QByteArray data = tcpDumpProcess_ptr_ -> readAllStandardOutput();
    QString output = QString::fromUtf8(data);

    QString extended_output = packetAnalyzer_ptr_ -> analyze( output );

    emit tcppackage_signal( output );
    emit classifyPacket_signal ( extended_output );
}

void TcpDumpManager::processError( QProcess::ProcessError error )
{
    qDebug() << "Error: " << error;
}

void TcpDumpManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Tcpdump process finished with exit code:" << exitCode << "and exit status:" << exitStatus;
}

void TcpDumpManager::onProcessStarted()
{
    qDebug() << "Tcpdump process started successfully.";
}

TcpDumpManager::~TcpDumpManager()
{
    if ( tcpDumpProcess_ptr_ ) {
        disconnect(tcpDumpProcess_ptr_.get(), &QProcess::readyReadStandardOutput, this, &TcpDumpManager::readTcpDumpOutput);
        disconnect(tcpDumpProcess_ptr_.get(), &QProcess::errorOccurred, this, &TcpDumpManager::processError);
        disconnect(tcpDumpProcess_ptr_.get(), &QProcess::started, this, &TcpDumpManager::onProcessStarted);
        disconnect(tcpDumpProcess_ptr_.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &TcpDumpManager::onProcessFinished);
    }
}
