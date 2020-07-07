#pragma once

#include <QCoreApplication>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catApp)

class ConduitHandler;
class ConsoleApplication : public QCoreApplication
{
    Q_OBJECT
public:
                            ConsoleApplication(int& argc, char** argv);
                            ~ConsoleApplication() override;
    int                     run();
private slots:
    void                    startConduitHandlers();
private:
    void                    init();
    void                    close();
    QStringList             enumConduits();
    void                    stopConduitHandlers();

    QList<ConduitHandler*>  conduitHandlers_;
};

