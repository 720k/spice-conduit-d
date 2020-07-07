#include "ConsoleApplication.h"
#include "ConduitThread.h"
#include <QDebug>
#include <QTimer>
#include <QDeadlineTimer>
#include <chrono>
Q_LOGGING_CATEGORY(catApp,"App")

ConsoleApplication::ConsoleApplication(int &argCount, char **argValues) : QCoreApplication(argCount,argValues) {
}



int ConsoleApplication::run() {
    init();
    int code = exec();
    close();
    return code;
}

void ConsoleApplication::startConduitThreads() {
    qCDebug(catApp) << "Start Conduit Handlers";
    for (auto conduitPortName : enumConduits())     conduitThreads_ << new ConduitThread(conduitPortName);
}

QStringList ConsoleApplication::enumConduits() {
    // enum conduit wit WindowsAPI
    return QStringList() << "io.bplater.data.0"; //#HARDCODED: conduit list
}

void ConsoleApplication::init() {
    QTimer::singleShot(0, this, &ConsoleApplication::startConduitThreads);
}

void ConsoleApplication::stopConduitThreads() {
    using namespace std::chrono_literals;
    for (auto handler : conduitThreads_)   handler->quit();
    for (auto handler : conduitThreads_)   handler->wait(QDeadlineTimer(1s));
    qDeleteAll(conduitThreads_);
}

void ConsoleApplication::close() {
    stopConduitThreads();
}




