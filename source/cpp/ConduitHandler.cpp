#include "ConduitHandler.h"
#include <QDebug>
Q_LOGGING_CATEGORY(catConduitHandler,"ConduitHandler")

ConduitHandler::ConduitHandler(const QString &portName, QObject *parent) : QObject(parent), portName_(portName) {
    systemPortName_ = QString( R"'(\\.\global\%1)'" ).arg(portName_);
}

ConduitHandler::~ConduitHandler() {
    // flush everything
    // close everything
}
