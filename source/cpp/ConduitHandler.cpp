#include "ConduitHandler.h"
#include <QDebug>
#include <QLocalSocket>
Q_LOGGING_CATEGORY(catConduitHandler,"ConduitHandler")

//#TODO: find alternative to MACRO, maybe template folding expression
#define DBG qCDebug(catConduitHandler).noquote() << portName_ << ">"

ConduitHandler::ConduitHandler(const QString &portName, QObject *parent) : QObject(parent), portName_(portName) {
    systemPort_.setPortName(  QString( R"'(\\.\global\%1)'" ).arg(portName_) );
}

void ConduitHandler::stop() {
    localServer_.stop();
    closeSystemPort();
}

void ConduitHandler::systemPortReadyRead() {
    while (systemPort_.bytesAvailable()) {
        auto data = systemPort_.readAll();
        localServer_.writeData(data);
    }
}

void ConduitHandler::localServerReadyRead(QByteArray data) {
    systemPort_.write(data);
    systemPort_.flush();
}

ConduitHandler::~ConduitHandler() {
    stop();
}

bool ConduitHandler::openSystemPort() {
    if (systemPort_.open(QIODevice::ReadWrite))     {
        systemPort_.read(nullptr,0);
        DBG << "SystemPort Connected:" << systemPort_.portName();
    }  else {
        DBG << "SystemPort FAILED to connect: " << systemPort_.portName();
        return false;
    }
    return true;
}

bool ConduitHandler::startlocalServer() {
    return localServer_.start(portName_);
}

bool ConduitHandler::start() {
    if (!openSystemPort())      return false;
    if (!startlocalServer()) {
        closeSystemPort();
        return false;
    }
    // bind in&out
    //connect(&systemPort_, QOverload<WSerialPort::SerialPortError>::of(&WSerialPort::error), this, &MainWindow::error);
    connect(&systemPort_, &WSerialPort::readyRead,          this, &ConduitHandler::systemPortReadyRead);
    connect(&localServer_, &NetworkLocalServer::dataReady,  this, &ConduitHandler::localServerReadyRead);
    return true;
}

void ConduitHandler::closeSystemPort() {
   if (systemPort_.isOpen()) {
       systemPort_.flush();
       systemPort_.close();
   }
}
