/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Copyright (C) 2012 Andre Hartmann <aha_1980@gmx.de>
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

#include "WSerialPort_p.h"
#include "qwinoverlappedionotifier_p.h"


#include <QtCore/qcoreevent.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qvector.h>
#include <QtCore/qtimer.h>
#include <algorithm>
#include <QDebug>

//#ifndef CTL_CODE
//#  define CTL_CODE(DeviceType, Function, Method, Access) ( \
//    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
//    )
//#endif

//#ifndef FILE_DEVICE_SERIAL_PORT
//#  define FILE_DEVICE_SERIAL_PORT  27
//#endif

//#ifndef METHOD_BUFFERED
//#  define METHOD_BUFFERED  0
//#endif

//#ifndef FILE_ANY_ACCESS
//#  define FILE_ANY_ACCESS  0x00000000
//#endif


bool WSerialPortPrivate::open(QIODevice::OpenMode mode)
{
    DWORD desiredAccess = 0;

    if (mode & QIODevice::ReadOnly)        desiredAccess |= GENERIC_READ;
    if (mode & QIODevice::WriteOnly)        desiredAccess |= GENERIC_WRITE;

    handle = ::CreateFile(reinterpret_cast<const wchar_t*>(systemLocation.utf16()),
                              desiredAccess, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

    if (handle == INVALID_HANDLE_VALUE) {
        setError(getSystemError());
        return false;
    }

    if (initialize(mode))        return true;

    ::CloseHandle(handle);
    return false;
}

void WSerialPortPrivate::close() {
    ::CancelIo(handle);

    delete notifier;
    notifier = nullptr;

    delete startAsyncWriteTimer;
    startAsyncWriteTimer = nullptr;

//    communicationStarted = false;
    readStarted = false;
    writeStarted = false;
    writeBuffer.clear();

    ::CloseHandle(handle);
    handle = INVALID_HANDLE_VALUE;
}

bool WSerialPortPrivate::flush()    {
    return _q_startAsyncWrite();
}

//bool WSerialPortPrivate::clear(WSerialPort::Directions directions)
//{
//    DWORD flags = 0;
//    if (directions & WSerialPort::Input)
//        flags |= PURGE_RXABORT | PURGE_RXCLEAR;
//    if (directions & WSerialPort::Output)
//        flags |= PURGE_TXABORT | PURGE_TXCLEAR;
//    if (!::PurgeComm(handle, flags)) {
//        setError(getSystemError());
//        return false;
//    }

//    // We need start async read because a reading can be stalled. Since the
//    // PurgeComm can abort of current reading sequence, or a port is in hardware
//    // flow control mode, or a port has a limited read buffer size.
//    if (directions & WSerialPort::Input)
//        startAsyncCommunication();

//    return true;
//}


bool WSerialPortPrivate::waitForReadyRead(int msecs)
{
    if (!writeStarted && !_q_startAsyncWrite())  return false;

    const qint64 initialReadBufferSize = buffer.size();
    qint64 currentReadBufferSize = initialReadBufferSize;

    QDeadlineTimer deadline(msecs);

    do {
        const OVERLAPPED *overlapped = waitForNotified(deadline);
        if (!overlapped)        return false;

        if (overlapped == &readCompletionOverlapped) {
            const qint64 readBytesForOneReadOperation = qint64(buffer.size()) - currentReadBufferSize;
            if (readBytesForOneReadOperation == WSERIALPORT_BUFFERSIZE) {
                currentReadBufferSize = buffer.size();
            } else if (readBytesForOneReadOperation == 0) {
                if (initialReadBufferSize != currentReadBufferSize)     return true;
            } else {
                return true;
            }
        }

    } while (!deadline.hasExpired());

    return false;
}

bool WSerialPortPrivate::waitForBytesWritten(int msecs)
{
    if (writeBuffer.isEmpty() && writeChunkBuffer.isEmpty())        return false;

    if (!writeStarted && !_q_startAsyncWrite())        return false;

    QDeadlineTimer deadline(msecs);

    for (;;) {
        const OVERLAPPED *overlapped = waitForNotified(deadline);
        if (!overlapped)    return false;
        if (overlapped == &writeCompletionOverlapped)   return true;
    }
    return false;
}


//bool WSerialPortPrivate::completeAsyncCommunication(qint64 bytesTransferred)    {
//    communicationStarted = false;
//    if (bytesTransferred == qint64(-1))        return false;
//    return startAsyncRead();
//}

bool WSerialPortPrivate::completeAsyncRead(qint64 bytesTransferred)
{
    if (bytesTransferred == qint64(-1)) {
        readStarted = false;
        return false;
    }
    if (bytesTransferred > 0)   buffer.append(readChunkBuffer.constData(), bytesTransferred);

    readStarted = false;

    bool result = startAsyncRead();
//    if (bytesTransferred == WSERIALPORT_BUFFERSIZE || queuedBytesCount(WSerialPort::Input) > 0) {
//        result = startAsyncRead();
//    } else {
//        result = startAsyncCommunication();
//    }

    if (bytesTransferred > 0)       emitReadyRead();

    return result;
}

bool WSerialPortPrivate::completeAsyncWrite(qint64 bytesTransferred)
{
    Q_Q(WSerialPort);

    if (writeStarted) {
        if (bytesTransferred == qint64(-1)) {
            writeChunkBuffer.clear();
            writeStarted = false;
            return false;
        }
        Q_ASSERT(bytesTransferred == writeChunkBuffer.size());
        writeChunkBuffer.clear();
        emit q->bytesWritten(bytesTransferred);
        writeStarted = false;
    }

    return _q_startAsyncWrite();
}

//bool WSerialPortPrivate::startAsyncCommunication()
//{
//    if (communicationStarted)       return true;

//    ::ZeroMemory(&communicationOverlapped, sizeof(communicationOverlapped));
//    if (!::WaitCommEvent(handle, &triggeredEventMask, &communicationOverlapped)) {
//        WSerialPortErrorInfo error = getSystemError();
//        if (error.errorCode != WSerialPort::NoError) {
//            if (error.errorCode == WSerialPort::PermissionError)
//                error.errorCode = WSerialPort::ResourceError;
//            setError(error);
//            return false;
//        }
//    }
//    communicationStarted = true;
//    return true;
//}

bool WSerialPortPrivate::startAsyncRead()       {
    if (readStarted)        return true;

    qint64 bytesToRead = WSERIALPORT_BUFFERSIZE;

    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - buffer.size())) {
        bytesToRead = readBufferMaxSize - buffer.size();
        if (bytesToRead <= 0) {
            // Buffer is full. User must read data from the buffer
            // before we can read more from the port.
            return false;
        }
    }

    Q_ASSERT(int(bytesToRead) <= readChunkBuffer.size());
    ::ZeroMemory(&readCompletionOverlapped, sizeof(readCompletionOverlapped));
    qDebug() << "::ReadFile";
    if (::ReadFile(handle, readChunkBuffer.data(), bytesToRead, nullptr, &readCompletionOverlapped)) {
        readStarted = true;
        return true;
    }

    WSerialPortErrorInfo error = getSystemError();
    if (error.errorCode != WSerialPort::NoError) {
        if (error.errorCode == WSerialPort::PermissionError)    error.errorCode = WSerialPort::ResourceError;
        if (error.errorCode != WSerialPort::ResourceError)      error.errorCode = WSerialPort::ReadError;
        setError(error);
        return false;
    }
    readStarted = true;
    return true;
}

bool WSerialPortPrivate::_q_startAsyncWrite()
{
    if (writeBuffer.isEmpty() || writeStarted)      return true;

    writeChunkBuffer = writeBuffer.read();
    ::ZeroMemory(&writeCompletionOverlapped, sizeof(writeCompletionOverlapped));
    if (!::WriteFile(handle, writeChunkBuffer.constData(), writeChunkBuffer.size(), nullptr, &writeCompletionOverlapped)) {
        WSerialPortErrorInfo error = getSystemError();
        if (error.errorCode != WSerialPort::NoError) {
            if (error.errorCode != WSerialPort::ResourceError)  error.errorCode = WSerialPort::WriteError;
            setError(error);
            return false;
        }
    }
    writeStarted = true;
    return true;
}

void WSerialPortPrivate::_q_notified(DWORD numberOfBytes, DWORD errorCode, OVERLAPPED *overlapped)
{
    qDebug() << "q_notified of :" << overlapped;
    const WSerialPortErrorInfo error = getSystemError(errorCode);
    if (error.errorCode != WSerialPort::NoError) {
        setError(error);
        return;
    }
    /*if (overlapped == &communicationOverlapped)         completeAsyncCommunication(numberOfBytes);
    else*/
    if (overlapped == &readCompletionOverlapped)        completeAsyncRead(numberOfBytes);
    else if (overlapped == &writeCompletionOverlapped)  completeAsyncWrite(numberOfBytes);
    else                                                Q_ASSERT(!"Unknown OVERLAPPED activated");
}

void WSerialPortPrivate::emitReadyRead()    {
    Q_Q(WSerialPort);
    emit q->readyRead();
}

qint64 WSerialPortPrivate::writeData(const char *data, qint64 maxSize)  {
    Q_Q(WSerialPort);

    writeBuffer.append(data, maxSize);

    if (!writeBuffer.isEmpty() && !writeStarted) {
        if (!startAsyncWriteTimer) {
            startAsyncWriteTimer = new QTimer(q);
            QObjectPrivate::connect(startAsyncWriteTimer, &QTimer::timeout, this, &WSerialPortPrivate::_q_startAsyncWrite);
            startAsyncWriteTimer->setSingleShot(true);
        }
        if (!startAsyncWriteTimer->isActive())      startAsyncWriteTimer->start();
    }
    return maxSize;
}

OVERLAPPED *WSerialPortPrivate::waitForNotified(QDeadlineTimer deadline)    {
    OVERLAPPED *overlapped = notifier->waitForAnyNotified(deadline);
    if (!overlapped) {
        setError(getSystemError(WAIT_TIMEOUT));
        return nullptr;
    }
    return overlapped;
}

//qint64 WSerialPortPrivate::queuedBytesCount(WSerialPort::Direction direction) const
//{
//    COMSTAT comstat;
//    if (::ClearCommError(handle, nullptr, &comstat) == 0)        return -1;
//    return (direction == WSerialPort::Input) ? comstat.cbInQue : ((direction == WSerialPort::Output) ? comstat.cbOutQue : -1);
//}

inline bool WSerialPortPrivate::initialize(QIODevice::OpenMode mode)    {
    Q_Q(WSerialPort);

    notifier = new QWinOverlappedIoNotifier(q);
    QObjectPrivate::connect(notifier, &QWinOverlappedIoNotifier::notified, this, &WSerialPortPrivate::_q_notified);
    notifier->setHandle(handle);
    notifier->setEnabled(true);

    return true;
}

WSerialPortErrorInfo WSerialPortPrivate::getSystemError(int systemErrorCode) const
{
    if (systemErrorCode == -1)      systemErrorCode = ::GetLastError();

    WSerialPortErrorInfo error;
    error.errorString = qt_error_string(systemErrorCode);

    switch (systemErrorCode) {
    case ERROR_SUCCESS:
        error.errorCode = WSerialPort::NoError;
        break;
    case ERROR_IO_PENDING:
        error.errorCode = WSerialPort::NoError;
        break;
    case ERROR_MORE_DATA:
        error.errorCode = WSerialPort::NoError;
        break;
    case ERROR_FILE_NOT_FOUND:
        error.errorCode = WSerialPort::DeviceNotFoundError;
        break;
    case ERROR_PATH_NOT_FOUND:
        error.errorCode = WSerialPort::DeviceNotFoundError;
        break;
    case ERROR_INVALID_NAME:
        error.errorCode = WSerialPort::DeviceNotFoundError;
        break;
    case ERROR_ACCESS_DENIED:
        error.errorCode = WSerialPort::PermissionError;
        break;
    case ERROR_INVALID_HANDLE:
        error.errorCode = WSerialPort::ResourceError;
        break;
    case ERROR_INVALID_PARAMETER:
        error.errorCode = WSerialPort::UnsupportedOperationError;
        break;
    case ERROR_BAD_COMMAND:
        error.errorCode = WSerialPort::ResourceError;
        break;
    case ERROR_DEVICE_REMOVED:
        error.errorCode = WSerialPort::ResourceError;
        break;
    case ERROR_OPERATION_ABORTED:
        error.errorCode = WSerialPort::ResourceError;
        break;
    case WAIT_TIMEOUT:
        error.errorCode = WSerialPort::TimeoutError;
        break;
    default:
        error.errorCode = WSerialPort::UnknownError;
        break;
    }
    return error;
}

WSerialPort::Handle WSerialPort::handle() const {
    Q_D(const WSerialPort);
    return d->handle;
}

