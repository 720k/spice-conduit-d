#pragma once

#include "QtService/QtService"
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(catServiceApplication)

class ConduitThread;  //fwd dec

class ServiceApplication : public QtService::Service
{
    Q_OBJECT
public:
    explicit ServiceApplication(int &argc, char **argv);

protected:
    bool            preStart() override;
    CommandResult   onStart() override;
    CommandResult   onStop(int &exitCode) override;
    CommandResult   onReload() override;
    CommandResult   onPause() override;
    CommandResult   onResume() override;
private slots:
    void                    startConduitThreads();
private:
    void                    init();
    void                    close();
    QStringList             enumConduits();
    void                    stopConduitThreads();

    QList<ConduitThread*>   conduitThreads_;
};

