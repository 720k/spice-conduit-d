#pragma once

#include <QCoreApplication>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catApp)

class ConduitThread;
class ConsoleApplication : public QCoreApplication
{
    Q_OBJECT
public:
                            ConsoleApplication(int& argc, char** argv);
    int                     run();
private slots:
    void                    startConduitThreads();
private:
    void                    init();
    void                    close();
    QStringList             enumConduits();
    void                    stopConduitThreads();

    QList<ConduitThread*>   conduitThreads_;
};

