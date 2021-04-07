#include "NetworkLocalServer.h"
#include "ConsoleColors.h"
#include <QDebug>
#include <QFile>
Q_LOGGING_CATEGORY(catNetworkLocalServer,"NetworkLocalServer")
//#TODO: find alternative to MACRO, maybe template folding expression
#define DBG qCDebug(catNetworkLocalServer).noquote().nospace() << serverName() << "> "

NetworkLocalServer::NetworkLocalServer(QObject *parent) : QLocalServer(parent) {
    connect(this, &QLocalServer::newConnection, this, &NetworkLocalServer::newConnectionAvailable);
    setSocketOptions(QLocalServer::WorldAccessOption);
}

void NetworkLocalServer::print(QString str, const QByteArray &data) {
    QString prefix = QString("%1 %2:").arg(str).arg(data.size());
    prefix += QString(30-prefix.count(),' ');
    auto len = data.left(4).toHex(' ') + " ";
    auto id = data.mid(4,4).toHex(' ') + " ";
    auto other = data.mid(8,8).toHex(' ');
    DBG << prefix << cYLW  << len << cORG << id << cRST << other;
}


void NetworkLocalServer::writeData(QByteArray data) {
    print(objectName()+" Write", data);
    if (socket_) {
        socket_->write(data);
        socket_->flush();
    } else {
        DBG << "Socket is close, sink data";
    }
}

void NetworkLocalServer::deleteSocket() {
    if (socket_) {
        socket_->flush();
        socket_->disconnectFromServer();
        socket_->deleteLater();
        socket_=nullptr;
    }
}

void NetworkLocalServer::stop() {
    deleteSocket();
    close();
}

bool NetworkLocalServer::start(const QString &port) {
    if (listen(port)) {
        DBG << "is listening";
    } else {
        DBG << " FAILED to listen";
        return false;
    }
    return true;
}

void NetworkLocalServer::newConnectionAvailable() {
    if (socket_) {
        DBG << "Sorry, I can accept only one connection";
        return;
    }
    socket_ = nextPendingConnection();
    connect(socket_, &QLocalSocket::stateChanged, this, &NetworkLocalServer::onSocketStateChanged);
    connect(socket_, &QLocalSocket::errorOccurred, this, &NetworkLocalServer::onSocketErrorOccurred);
    connect(socket_, &QLocalSocket::readyRead,      this, &NetworkLocalServer::onSocketData);
    DBG << "New connection accepted";
}

void NetworkLocalServer::onSocketErrorOccurred(QLocalSocket::LocalSocketError socketError) {  Q_UNUSED(socketError)
    qDebug() << objectName() << "Socket error: " << socket_->errorString();
    deleteSocket();
}

void NetworkLocalServer::onSocketStateChanged(QLocalSocket::LocalSocketState socketState) {
    if (socketState == QLocalSocket::UnconnectedState) {
        deleteSocket();
    }
    if (socketState == QLocalSocket::ConnectedState) {
    }
    emit socketStateChanged(socketState);
}

void NetworkLocalServer::onSocketData() {
    while (socket_->bytesAvailable()) {
        auto data = socket_->readAll();
        print(objectName()+" Read",data );
        emit dataReady(data);
    }
}
