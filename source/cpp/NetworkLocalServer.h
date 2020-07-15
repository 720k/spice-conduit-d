#pragma once
#include <QLocalServer>
#include <QLocalSocket>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catNetworkLocalServer)

class NetworkLocalServer : public QLocalServer
{
    Q_OBJECT
public:
                        NetworkLocalServer(QObject *parent = nullptr);

                        void deleteSocket();

signals:
    void                socketStateChanged(QLocalSocket::LocalSocketState socketState);
    void                dataReady(QByteArray data);
public slots:
    void                writeData(QByteArray data);
    void                stop();
    bool start(const QString& port);
private slots:
    void                newConnectionAvailable();
    void                onSocketErrorOccurred(QLocalSocket::LocalSocketError socketError);
    void                onSocketStateChanged(QLocalSocket::LocalSocketState socketState);
    void                onSocketData();

private:
    void                print(QString str, const QByteArray& data=QByteArray());

    QLocalSocket*       socket_=nullptr;

};

