#ifndef CONDUITHANDLER_H
#define CONDUITHANDLER_H

#include <QThread>
#include <QLoggingCategory>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(catConduitHandler)

class ConduitHandler : public QThread
{
    Q_OBJECT
public:
    ConduitHandler(QObject *parent = nullptr);
    ConduitHandler(const QString& portName, QObject *parent = nullptr);
    ~ConduitHandler() override;

    void run() override;
private:

    QString         portName_;
};

#endif // CONDUITHANDLER_H
