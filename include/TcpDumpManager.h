#pragma once

#include <QObject>
#include <QProcess>

#include <memory>

class TcpDumpManager : public QObject
{
    Q_OBJECT
public:
    explicit TcpDumpManager(QObject *parent = nullptr);
    Q_INVOKABLE void startTcpDump();

    ~TcpDumpManager();
public slots:
    void readTcpDumpOutput();
    void processError( QProcess::ProcessError error );

    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

signals:
    void tcppackage_signal( QString new_package );

private:
    std::unique_ptr< QProcess > tcpDumpProcess_ptr_;
};
