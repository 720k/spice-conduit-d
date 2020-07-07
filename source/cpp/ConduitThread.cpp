#include "ConduitThread.h"
#include "ConduitHandler.h"

#include <QDebug>
Q_LOGGING_CATEGORY(catConduitThread,"ConduitThread")


ConduitThread::ConduitThread(QObject *parent) : QThread(parent) {
    start();
}

ConduitThread::ConduitThread(const QString &portName, QObject *parent) : ConduitThread(parent) {
    portName_ = portName;
    qCDebug(catConduitThread).noquote() << portName_ << " +++ Constructor";
    handler_ = new ConduitHandler(portName_);
    handler_->moveToThread(this);
    start();
}

ConduitThread::~ConduitThread() {
    if (handler_) {
        delete handler_;
        handler_ = nullptr;
    }
    qCDebug(catConduitThread).noquote() << portName_ << " --- Destructor";
}

void ConduitThread::run() {
    qCDebug(catConduitThread).noquote() << portName_ << " > Run";
    exec();
    qCDebug(catConduitThread).noquote() << portName_ << " > Exit";
}
