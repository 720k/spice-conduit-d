#pragma once

#include "WSerialPort/WSerialPort.h"
#include "NetworkLocalServer.h"
#include <QObject>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catConduitHandler)


class ConduitHandler : public QObject
{
    Q_OBJECT
public:
    explicit            ConduitHandler(const QString& portName, QObject *parent = nullptr);
                        ~ConduitHandler() override;


                        bool start();
                        void stop();
private slots:
    void                systemPortReadyRead();
    void                localServerReadyRead(QByteArray data);
signals:
private:
    void                closeSystemPort();
    bool                openSystemPort();
    bool                startlocalServer();

    QString             portName_;
    WSerialPort         systemPort_;
    NetworkLocalServer  localServer_;
};

