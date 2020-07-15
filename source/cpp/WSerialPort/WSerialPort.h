/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#pragma once
#include <QtCore/qiodevice.h>


//class WSerialPortInfo;
class WSerialPortPrivate;

class WSerialPort : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WSerialPort)
    Q_PROPERTY(SerialPortError error READ error RESET clearError NOTIFY error)
#if defined(Q_OS_WIN32)
    typedef void* Handle;
#else
    typedef int Handle;
#endif

public:

//    enum Direction  {
//        Input = 1,
//        Output = 2,
//        AllDirections = Input | Output
//    };
//    Q_FLAG(Direction)
//    Q_DECLARE_FLAGS(Directions, Direction)

    enum SerialPortError {
        NoError,
        DeviceNotFoundError,
        PermissionError,
        OpenError,
        ParityError,
        FramingError,
        BreakConditionError,
        WriteError,
        ReadError,
        ResourceError,
        UnsupportedOperationError,
        UnknownError,
        TimeoutError,
        NotOpenError
    };
    Q_ENUM(SerialPortError)

    explicit WSerialPort(QObject *parent = nullptr);
    explicit WSerialPort(const QString &name, QObject *parent = nullptr);
//    explicit WSerialPort(const WSerialPortInfo &info, QObject *parent = nullptr);
    virtual ~WSerialPort() override;

    void setPortName(const QString &name);
    QString portName() const;

//    void setPort(const WSerialPortInfo &info);

    bool open(OpenMode mode) override;
    void close() override;

    bool flush();
//    bool clear(Directions directions = AllDirections);
    bool atEnd() const override; // ### Qt6: remove me

    SerialPortError error() const;
    void clearError();

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    bool isSequential() const override;

    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;
    bool canReadLine() const override;

    bool waitForReadyRead(int msecs = 30000) override;
    bool waitForBytesWritten(int msecs = 30000) override;

    Handle handle() const;

Q_SIGNALS:
#if QT_DEPRECATED_SINCE(5, 8)
    void error(WSerialPort::SerialPortError serialPortError);
#endif
    void errorOccurred(WSerialPort::SerialPortError error);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 readLineData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    // ### Qt6: remove me.
    WSerialPortPrivate * const d_dummy;

    Q_DISABLE_COPY(WSerialPort)

#if defined(Q_OS_WIN32)
    Q_PRIVATE_SLOT(d_func(), bool _q_startAsyncWrite())
    //Q_PRIVATE_SLOT(d_func(), void _q_notified(quint32, quint32, OVERLAPPED*))
#endif
};

//Q_DECLARE_OPERATORS_FOR_FLAGS(WSerialPort::Directions)
