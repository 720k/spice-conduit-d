#include "ConsoleApplication.h"
#include "ConduitThread.h"
#include <QDebug>
#include <QTimer>
#include <QDeadlineTimer>
#include <chrono>
Q_LOGGING_CATEGORY(catConsoleApplication,"ConsoleApplication")

ConsoleApplication::ConsoleApplication(int &argCount, char **argValues) : QCoreApplication(argCount,argValues) {
}



int ConsoleApplication::run() {
    init();
    int code = exec();
    close();
    return code;
}

void ConsoleApplication::startConduitThreads() {
    qCDebug(catConsoleApplication) << "Start Conduit Handlers";
    for (auto conduitPortName : enumConduits())     conduitThreads_ << new ConduitThread(conduitPortName);
    for (auto thread : conduitThreads_)            thread->start();
}

QStringList ConsoleApplication::enumConduits() {
    // enum conduit wit WindowsAPI
    return QStringList() << "io.bplayer.data.0"; //#HARDCODED: conduit list
}

void ConsoleApplication::init() {
    QTimer::singleShot(0, this, &ConsoleApplication::startConduitThreads);
}

void ConsoleApplication::stopConduitThreads() {
    using namespace std::chrono_literals;
    for (auto thread : conduitThreads_)   thread->quit();
    for (auto thread : conduitThreads_)   thread->wait(QDeadlineTimer(1s));
    conduitThreads_.clear();
}

void ConsoleApplication::close() {
    stopConduitThreads();
}




