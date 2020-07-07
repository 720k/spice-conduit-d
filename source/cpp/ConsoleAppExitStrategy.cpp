#include "ConsoleAppExitStrategy.h"

#include <iostream>
#include <chrono>
#include <QCoreApplication>
#include <QDebug>

Q_LOGGING_CATEGORY(catConsoleAppExitStrategy, "catConsoleAppExitStrategy")

ConsoleAppExitStrategy::ConsoleAppExitStrategy(QObject *parent) : QObject(parent) {
    exitFn_ = std::async(std::launch::async, [&]() { std::getchar(); exitFlag_=true; } );
    using namespace std::chrono_literals;
    timerId_= startTimer(300ms);
    qCDebug(catConsoleAppExitStrategy) << "*** press <ENTER> to Quit!";
}

ConsoleAppExitStrategy::~ConsoleAppExitStrategy() {
    exitFn_.wait();
}

void ConsoleAppExitStrategy::timerEvent(QTimerEvent *event) {           Q_UNUSED(event)
    if (exitFlag_) {
        killTimer(timerId_);
        qApp->quit();
    }
}
