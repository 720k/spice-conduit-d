#pragma once

#include "WSerialPort/WSerialPort.h"
#include <QObject>
#include <QLocalServer>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catConduitHandler)

class ConduitHandler : public QObject
{
    Q_OBJECT
public:
    explicit            ConduitHandler(const QString& portName, QObject *parent = nullptr);
                        ~ConduitHandler() override;


signals:
private:
    WSerialPort     systemPort_;
    QLocalServer    localServer_;
    QLocalSocket*   localPort_=nullptr;
    QString         portName_;
    QString         systemPortName_;
};

