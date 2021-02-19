#include "ServiceApplication.h"
#include "ConduitThread.h"

#include <QDebug>
#include <QTimer>
#include <QDeadlineTimer>
#include <chrono>
Q_LOGGING_CATEGORY(catServiceApplication,"ServiceApplication")

ServiceApplication::ServiceApplication(int &argc, char **argv) : QtService::Service(argc,argv) {
}

bool ServiceApplication::preStart() {
    qCDebug(catServiceApplication) << Q_FUNC_INFO  << " running with backend:" << backend();
    return true;
}

QtService::Service::CommandResult ServiceApplication::onStart() {
    // cannot find any conduit port => exit
    startConduitThreads();
    return CommandResult::Completed;
}

QtService::Service::CommandResult ServiceApplication::onStop(int &exitCode) {   Q_UNUSED(exitCode)
    stopConduitThreads();
    return CommandResult::Completed;
}

QtService::Service::CommandResult ServiceApplication::onReload() {
    stopConduitThreads();
    startConduitThreads();
    return CommandResult::Completed;
}

QtService::Service::CommandResult ServiceApplication::onPause() {
    return CommandResult::Completed;
}

QtService::Service::CommandResult ServiceApplication::onResume() {
    return CommandResult::Completed;
}

void ServiceApplication::startConduitThreads() {
    qCDebug(catServiceApplication) << "Start Conduit Handlers";
    for (auto conduitPortName : enumConduits())     conduitThreads_ << new ConduitThread(conduitPortName);
    for (auto thread : conduitThreads_)            thread->start();
}

void ServiceApplication::init() {
    QTimer::singleShot(0, this, &ServiceApplication::startConduitThreads);
}

void ServiceApplication::close() {
    stopConduitThreads();
}

QStringList ServiceApplication::enumConduits() {
    // enum conduit wit WindowsAPI
    return QStringList() << "io.bplayer.data.0"; //#HARDCODED: conduit list
}

void ServiceApplication::stopConduitThreads() {
    using namespace std::chrono_literals;
    for (auto thread : conduitThreads_)   thread->quit();
    for (auto thread : conduitThreads_)   thread->wait(QDeadlineTimer(300ms));
    conduitThreads_.clear();
}

