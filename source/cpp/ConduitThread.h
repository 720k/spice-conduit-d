#pragma once

#include <QThread>
#include <QLoggingCategory>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(catConduitThread)

class ConduitHandler;

class ConduitThread : public QThread
{
    Q_OBJECT
public:
    ConduitThread(QObject *parent = nullptr);
    ConduitThread(const QString& portName, QObject *parent = nullptr);
    ~ConduitThread() override;

    void run() override;
private:

    QString         portName_;
    ConduitHandler* handler_=nullptr;
};

