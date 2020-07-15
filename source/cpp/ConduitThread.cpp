#include "ConduitThread.h"
#include "ConduitHandler.h"

#include <QDebug>
Q_LOGGING_CATEGORY(catConduitThread,"ConduitThread")

//#TODO: find alternative to MACRO, maybe template folding expression
#define DBG qCDebug(catConduitThread).noquote() << portName_ << ">"

ConduitThread::ConduitThread(const QString &portName, QObject *parent) : QThread(parent) {
    portName_ = portName;
    DBG << "+++Constructor";
    connect(this, &ConduitThread::finished, this, &ConduitThread::deleteLater);
}

ConduitThread::~ConduitThread() {
    DBG<<"---Destructor";
}

void ConduitThread::run() {
    DBG<<"Run";
    handler_ = new ConduitHandler(portName_);
    if (handler_ && handler_->start())  {
        exec();
        handler_->stop();
        delete handler_;
        handler_ = nullptr;
    }
    DBG<<"Exit";
}
