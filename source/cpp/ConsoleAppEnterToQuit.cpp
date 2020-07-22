#include "ConsoleAppEnterToQuit.h"

#include <iostream>
#include <chrono>
#include <QCoreApplication>
#include <QDebug>

Q_LOGGING_CATEGORY(catConsoleAppEnterToQuit, "ConsoleAppEnterToQuit")

ConsoleAppEnterToQuit::ConsoleAppEnterToQuit(QObject *parent) : QObject(parent) {
    exitFn_ = std::async(std::launch::async, [&]() { std::getchar(); exitFlag_=true; } );
    using namespace std::chrono_literals;
    timerId_= startTimer(300ms);
    qCDebug(catConsoleAppEnterToQuit) << "*** press <ENTER> to Quit!";
}

ConsoleAppEnterToQuit::~ConsoleAppEnterToQuit() {
    exitFn_.wait();
}

void ConsoleAppEnterToQuit::timerEvent(QTimerEvent *event) {           Q_UNUSED(event)
    if (exitFlag_) {
        killTimer(timerId_);
        qApp->quit();
    }
}
