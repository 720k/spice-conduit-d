#include "ConsoleApplication.h"
#include "ConduitHandler.h"
#include <QDebug>
#include <QTimer>
#include <QDeadlineTimer>
#include <chrono>
Q_LOGGING_CATEGORY(catApp,"App")

ConsoleApplication::ConsoleApplication(int &argCount, char **argValues) : QCoreApplication(argCount,argValues) {
}

ConsoleApplication::~ConsoleApplication()   {
    close();
}


int ConsoleApplication::run() {
    init();
    return exec();
}

void ConsoleApplication::startConduitHandlers() {
    qCDebug(catApp) << "Start Conduit Handlers";
    for (const auto& conduitPortName : enumConduits()) {
        conduitHandlers_ << new ConduitHandler(conduitPortName);
    }
}

QStringList ConsoleApplication::enumConduits() {
    // enum conduit wit WindowsAPI
    return QStringList() << "io.bplater.data.0"; //#HARDCODED: conduit list
}

void ConsoleApplication::init() {
    QTimer::singleShot(0, this, &ConsoleApplication::startConduitHandlers);
}

void ConsoleApplication::stopConduitHandlers() {
    for (auto handler : conduitHandlers_)   handler->quit();
    using namespace std::chrono_literals;
    for (auto handler : conduitHandlers_)   handler->wait(QDeadlineTimer(1s));
    qDeleteAll(conduitHandlers_);
}

void ConsoleApplication::close() {
    stopConduitHandlers();
}




