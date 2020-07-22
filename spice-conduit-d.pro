QT -= gui
QT += core-private network network-private serialport serialport-private

CONFIG += c++17 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += ./WSerialPort

SOURCES += \
    source/cpp/ConduitHandler.cpp \
    source/cpp/ConduitThread.cpp \
    source/cpp/ConsoleAppEnterToQuit.cpp \
    source/cpp/ConsoleApplication.cpp \
    source/cpp/NetworkLocalServer.cpp \
    source/cpp/WSerialPort/WSerialPort.cpp \
    source/cpp/WSerialPort/qwinoverlappedionotifier.cpp \
    source/cpp/__main.cpp

win32 {
    SOURCES += source/cpp/WSerialPort/WSerialPort_win32.cpp
}

unix {
    SOURCES += source/cpp/WSerialPort/WSerialPort_unix.cpp
}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    source/cpp/ConduitHandler.h \
    source/cpp/ConduitThread.h \
    source/cpp/ConsoleAppEnterToQuit.h \
    source/cpp/ConsoleApplication.h \
    source/cpp/ConsoleColors.h \
    source/cpp/NetworkLocalServer.h \
    source/cpp/WSerialPort/WSerialPort.h \
    source/cpp/WSerialPort/WSerialPort_p.h \
    source/cpp/WSerialPort/qwinoverlappedionotifier_p.h

DISTFILES += \
    text/info.text



