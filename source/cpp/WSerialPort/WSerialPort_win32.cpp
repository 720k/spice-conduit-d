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
//#include "qwinoverlappedionotifier_p.h"


#include <QtCore/qcoreevent.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qvector.h>
#include <QtCore/qtimer.h>
#include <algorithm>
#include <QDebug>
#include <qt_windows.h>

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

class Overlapped final : public OVERLAPPED
{
    Q_DISABLE_COPY(Overlapped)
public:
    explicit Overlapped(WSerialPortPrivate *d);
    void clear();

    WSerialPortPrivate *dptr = nullptr;
};

Overlapped::Overlapped(WSerialPortPrivate *d) : dptr(d) {
}

void Overlapped::clear() {
    ::ZeroMemory(this, sizeof(OVERLAPPED));
}


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
    delete startAsyncWriteTimer;
    startAsyncWriteTimer = nullptr;

    if (readStarted) {
        readCompletionOverlapped->dptr = nullptr;
        ::CancelIoEx(handle, readCompletionOverlapped);
        // The object will be deleted in the I/O callback.
        readCompletionOverlapped = nullptr;
        readStarted = false;
    } else {
        delete readCompletionOverlapped;
        readCompletionOverlapped = nullptr;
    };

    if (writeStarted) {
        writeCompletionOverlapped->dptr = nullptr;
        ::CancelIoEx(handle, writeCompletionOverlapped);
        // The object will be deleted in the I/O callback.
        writeCompletionOverlapped = nullptr;
        writeStarted = false;
    } else {
        delete writeCompletionOverlapped;
        writeCompletionOverlapped = nullptr;
    }

    readBytesTransferred = 0;
    writeBytesTransferred = 0;
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


bool WSerialPortPrivate::waitForReadyRead(int msecs) {
    if (!writeStarted && !_q_startAsyncWrite())        return false;

    QDeadlineTimer deadline(msecs);

    do {
        if (readBytesTransferred <= 0) {
            const qint64 remaining = deadline.remainingTime();
            const DWORD result = ::SleepEx(remaining == -1 ? INFINITE : DWORD(remaining),TRUE);
            if (result != WAIT_IO_COMPLETION)   continue;
        }

        if (readBytesTransferred > 0) {
            readBytesTransferred = 0;
            return true;
        }
    } while (!deadline.hasExpired());

    setError(getSystemError(WAIT_TIMEOUT));
    return false;
}

bool WSerialPortPrivate::waitForBytesWritten(int msecs) {
    if (writeBuffer.isEmpty() && writeChunkBuffer.isEmpty())        return false;
    if (!writeStarted && !_q_startAsyncWrite())        return false;

    QDeadlineTimer deadline(msecs);

    do {
        if (writeBytesTransferred <= 0) {
            const qint64 remaining = deadline.remainingTime();
            const DWORD result = ::SleepEx(remaining == -1 ? INFINITE : DWORD(remaining),TRUE);
            if (result != WAIT_IO_COMPLETION)   continue;
        }

        if (writeBytesTransferred > 0) {
            writeBytesTransferred = 0;
            return true;
        }
    } while (!deadline.hasExpired());

    setError(getSystemError(WAIT_TIMEOUT));
    return false;
}


//bool WSerialPortPrivate::completeAsyncCommunication(qint64 bytesTransferred)    {
//    communicationStarted = false;
//    if (bytesTransferred == qint64(-1))        return false;
//    return startAsyncRead();
//}

bool WSerialPortPrivate::completeAsyncRead(qint64 bytesTransferred) {
    // Store the number of transferred bytes which are
    // required only in waitForReadyRead() method.
    readBytesTransferred = bytesTransferred;

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

bool WSerialPortPrivate::completeAsyncWrite(qint64 bytesTransferred)    {
    Q_Q(WSerialPort);

    // Store the number of transferred bytes which are
    // required only in waitForBytesWritten() method.
    writeBytesTransferred = bytesTransferred;

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
    if (!readCompletionOverlapped)  readCompletionOverlapped = new Overlapped(this);
    readCompletionOverlapped->clear();
    readStarted = true;
    qDebug() << "::ReadFile";
    if (!::ReadFileEx(handle,
                      readChunkBuffer.data(),
                      bytesToRead,
                      readCompletionOverlapped,
                      ioCompletionRoutine)) {
        readStarted = false;
        WSerialPortErrorInfo error = getSystemError();
        if (error.errorCode != WSerialPort::NoError) {
            if (error.errorCode == WSerialPort::PermissionError)
                error.errorCode = WSerialPort::ResourceError;
            if (error.errorCode != WSerialPort::ResourceError)
                error.errorCode = WSerialPort::ReadError;
            setError(error);
            return false;
        }
    }
    return true;
}

bool WSerialPortPrivate::_q_startAsyncWrite()
{
    if (writeBuffer.isEmpty() || writeStarted)
        return true;

    writeChunkBuffer = writeBuffer.read();

    if (!writeCompletionOverlapped)
        writeCompletionOverlapped = new Overlapped(this);

    writeCompletionOverlapped->clear();
    writeStarted = true;
    if (!::WriteFileEx(handle,
                       writeChunkBuffer.constData(),
                       writeChunkBuffer.size(),
                       writeCompletionOverlapped,
                       ioCompletionRoutine)) {
        writeStarted = false;
        WSerialPortErrorInfo error = getSystemError();
        if (error.errorCode != WSerialPort::NoError) {
            if (error.errorCode != WSerialPort::ResourceError)
                error.errorCode = WSerialPort::WriteError;
            setError(error);
            return false;
        }
    }
    return true;
}

void WSerialPortPrivate::handleNotification(DWORD bytesTransferred, DWORD errorCode,
                                            OVERLAPPED *overlapped)
{
    // This occurred e.g. after calling the CloseHandle() function,
    // just skip handling at all.
    if (handle == INVALID_HANDLE_VALUE)
        return;

    const WSerialPortErrorInfo error = getSystemError(errorCode);
    if (error.errorCode != WSerialPort::NoError) {
        setError(error);
        return;
    }


    if (overlapped == readCompletionOverlapped)
        completeAsyncRead(bytesTransferred);
    else if (overlapped == writeCompletionOverlapped)
        completeAsyncWrite(bytesTransferred);
    else
        Q_ASSERT(!"Unknown OVERLAPPED activated");
}



void WSerialPortPrivate::emitReadyRead()    {
    Q_Q(WSerialPort);
    emit q->readyRead();
}

qint64 WSerialPortPrivate::writeData(const char *data, qint64 maxSize)
{
    Q_Q(WSerialPort);

    writeBuffer.append(data, maxSize);

    if (!writeBuffer.isEmpty() && !writeStarted) {
        if (!startAsyncWriteTimer) {
            startAsyncWriteTimer = new QTimer(q);
            QObjectPrivate::connect(startAsyncWriteTimer, &QTimer::timeout, this, &WSerialPortPrivate::_q_startAsyncWrite);
            startAsyncWriteTimer->setSingleShot(true);
        }
        if (!startAsyncWriteTimer->isActive())
            startAsyncWriteTimer->start();
    }
    return maxSize;
}


//qint64 WSerialPortPrivate::queuedBytesCount(WSerialPort::Direction direction) const
//{
//    COMSTAT comstat;
//    if (::ClearCommError(handle, nullptr, &comstat) == 0)        return -1;
//    return (direction == WSerialPort::Input) ? comstat.cbInQue : ((direction == WSerialPort::Output) ? comstat.cbOutQue : -1);
//}

inline bool WSerialPortPrivate::initialize(QIODevice::OpenMode mode)    {
    Q_Q(WSerialPort);

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

void WSerialPortPrivate::ioCompletionRoutine(
        DWORD errorCode, DWORD bytesTransfered,
        OVERLAPPED *overlappedBase)
{
    const auto overlapped = static_cast<Overlapped *>(overlappedBase);
    if (overlapped->dptr) {
        overlapped->dptr->handleNotification(bytesTransfered, errorCode,
                                             overlappedBase);
    } else {
        delete overlapped;
    }
}

