#include "ConduitHandler.h"
#include <QDebug>
Q_LOGGING_CATEGORY(catConduitHandler,"ConduitHandler")


ConduitHandler::ConduitHandler(QObject *parent) : QThread(parent) {
    start();
}

ConduitHandler::ConduitHandler(const QString &portName, QObject *parent) : ConduitHandler(parent) {
    portName_ = portName;
    qCDebug(catConduitHandler).noquote() << portName_ << " +++ Constructor";
}

ConduitHandler::~ConduitHandler() {
    qCDebug(catConduitHandler).noquote() << portName_ << " --- Destructor";
}

void ConduitHandler::run() {
    qCDebug(catConduitHandler).noquote() << portName_ << " > Run";
       exec();
    qCDebug(catConduitHandler).noquote() << portName_ << " > Exit";
}
