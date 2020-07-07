/****************************************************************************
**
** Copyright (C) 2011-2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2011 Sergey Belyashov <Sergey.Belyashov@gmail.com>
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

#include "WSerialPort.h"
//#include "WSerialPortinfo.h"
//#include "WSerialPortinfo_p.h"

#include "WSerialPort_p.h"

#include <QtCore/qdebug.h>


WSerialPortErrorInfo::WSerialPortErrorInfo(WSerialPort::SerialPortError newErrorCode, const QString &newErrorString)
    : errorCode(newErrorCode)
    , errorString(newErrorString)
{
    if (errorString.isNull()) {
        switch (errorCode) {
        case WSerialPort::NoError:
            errorString = WSerialPort::tr("No error");
            break;
        case WSerialPort::OpenError:
            errorString = WSerialPort::tr("Device is already open");
            break;
        case WSerialPort::NotOpenError:
            errorString = WSerialPort::tr("Device is not open");
            break;
        case WSerialPort::TimeoutError:
            errorString = WSerialPort::tr("Operation timed out");
            break;
        case WSerialPort::ReadError:
            errorString = WSerialPort::tr("Error reading from device");
            break;
        case WSerialPort::WriteError:
            errorString = WSerialPort::tr("Error writing to device");
            break;
        case WSerialPort::ResourceError:
            errorString = WSerialPort::tr("Device disappeared from the system");
            break;
        default:
            // an empty string will be interpreted as "Unknown error"
            // from the QIODevice::errorString()
            break;
        }
    }
}

WSerialPortPrivate::WSerialPortPrivate()
#if defined(Q_OS_WIN32)
    : readChunkBuffer(WSERIALPORT_BUFFERSIZE, 0)
#endif
{
    writeBufferChunkSize = WSERIALPORT_BUFFERSIZE;
    readBufferChunkSize = WSERIALPORT_BUFFERSIZE;
}

void WSerialPortPrivate::setError(const WSerialPortErrorInfo &errorInfo)
{
    Q_Q(WSerialPort);

    error = errorInfo.errorCode;
    q->setErrorString(errorInfo.errorString);
    emit q->errorOccurred(error);
    emit q->error(error);
}

/*!
    \class WSerialPort

    \brief Provides functions to access serial ports.

    \reentrant
    \ingroup serialport-main
    \inmodule QtSerialPort
    \since 5.1

    You can get information about the available serial ports using the
    WSerialPortInfo helper class, which allows an enumeration of all the serial
    ports in the system. This is useful to obtain the correct name of the
    serial port you want to use. You can pass an object
    of the helper class as an argument to the setPort() or setPortName()
    methods to assign the desired serial device.

    After setting the port, you can open it in read-only (r/o), write-only
    (w/o), or read-write (r/w) mode using the open() method.

    \note The serial port is always opened with exclusive access
    (that is, no other process or thread can access an already opened serial port).

    Use the close() method to close the port and cancel the I/O operations.

    Having successfully opened, WSerialPort tries to determine the current
    configuration of the port and initializes itself. You can reconfigure the
    port to the desired setting using the setBaudRate(), setDataBits(),
    setParity(), setStopBits(), and setFlowControl() methods.

    There are a couple of properties to work with the pinout signals namely:
    WSerialPort::dataTerminalReady, WSerialPort::requestToSend. It is also
    possible to use the pinoutSignals() method to query the current pinout
    signals set.

    Once you know that the ports are ready to read or write, you can
    use the read() or write() methods. Alternatively the
    readLine() and readAll() convenience methods can also be invoked.
    If not all the data is read at once, the remaining data will
    be available for later as new incoming data is appended to the
    WSerialPort's internal read buffer. You can limit the size of the read
    buffer using setReadBufferSize().

    WSerialPort provides a set of functions that suspend the
    calling thread until certain signals are emitted. These functions
    can be used to implement blocking serial ports:

    \list

    \li waitForReadyRead() blocks calls until new data is available for
    reading.

    \li waitForBytesWritten() blocks calls until one payload of data has
    been written to the serial port.

    \endlist

    See the following example:

    \code
     int numRead = 0, numReadTotal = 0;
     char buffer[50];

     for (;;) {
         numRead  = serial.read(buffer, 50);

         // Do whatever with the array

         numReadTotal += numRead;
         if (numRead == 0 && !serial.waitForReadyRead())
             break;
     }
    \endcode

    If \l{QIODevice::}{waitForReadyRead()} returns \c false, the
    connection has been closed or an error has occurred.

    If an error occurs at any point in time, WSerialPort will emit the
    errorOccurred() signal. You can also call error() to find the type of
    error that occurred last.

    Programming with a blocking serial port is radically different from
    programming with a non-blocking serial port. A blocking serial port
    does not require an event loop and typically leads to simpler code.
    However, in a GUI application, blocking serial port should only be
    used in non-GUI threads, to avoid freezing the user interface.

    For more details about these approaches, refer to the
    \l {Qt Serial Port Examples}{example} applications.

    The WSerialPort class can also be used with QTextStream and QDataStream's
    stream operators (operator<<() and operator>>()). There is one issue to be
    aware of, though: make sure that enough data is available before attempting
    to read by using the operator>>() overloaded operator.

    \sa WSerialPortInfo
*/

/*!
    \enum WSerialPort::Direction

    This enum describes the possible directions of the data transmission.

    \note This enumeration is used for setting the baud rate of the device
    separately for each direction on some operating systems (for example,
    POSIX-like).

    \value Input            Input direction.
    \value Output           Output direction.
    \value AllDirections    Simultaneously in two directions.
*/


/*!
    \enum WSerialPort::SerialPortError

    This enum describes the errors that may be contained by the
    WSerialPort::error property.

    \value NoError              No error occurred.

    \value DeviceNotFoundError  An error occurred while attempting to
                                open an non-existing device.

    \value PermissionError      An error occurred while attempting to
                                open an already opened device by another
                                process or a user not having enough permission
                                and credentials to open.

    \value OpenError            An error occurred while attempting to open an
                                already opened device in this object.

    \value NotOpenError         This error occurs when an operation is executed
                                that can only be successfully performed if the
                                device is open. This value was introduced in
                                QtSerialPort 5.2.

    \value ParityError          Parity error detected by the hardware while
                                reading data. This value is obsolete. We strongly
                                advise against using it in new code.

    \value FramingError         Framing error detected by the hardware while
                                reading data. This value is obsolete. We strongly
                                advise against using it in new code.

    \value BreakConditionError  Break condition detected by the hardware on
                                the input line. This value is obsolete. We strongly
                                advise against using it in new code.

    \value WriteError           An I/O error occurred while writing the data.

    \value ReadError            An I/O error occurred while reading the data.

    \value ResourceError        An I/O error occurred when a resource becomes
                                unavailable, e.g. when the device is
                                unexpectedly removed from the system.

    \value UnsupportedOperationError The requested device operation is not
                                supported or prohibited by the running operating
                                system.

    \value TimeoutError         A timeout error occurred. This value was
                                introduced in QtSerialPort 5.2.

    \value UnknownError         An unidentified error occurred.
    \sa WSerialPort::error
*/



/*!
    Constructs a new serial port object with the given \a parent.
*/
WSerialPort::WSerialPort(QObject *parent) : QIODevice(*new WSerialPortPrivate, parent), d_dummy(nullptr)    {
}

/*!
    Constructs a new serial port object with the given \a parent
    to represent the serial port with the specified \a name.

    The name should have a specific format; see the setPort() method.
*/
WSerialPort::WSerialPort(const QString &name, QObject *parent) : WSerialPort (parent) {
    setPortName(name);
}


/*!
    Closes the serial port, if necessary, and then destroys object.
*/
WSerialPort::~WSerialPort() {
    if (isOpen())   close();
}

/*!
    Sets the \a name of the serial port.

    The name of the serial port can be passed as either a short name or
    the long system location if necessary.

    \sa portName(), WSerialPortInfo
*/
void WSerialPort::setPortName(const QString &name)  {
    Q_D(WSerialPort);
//    d->systemLocation = WSerialPortPrivate::portNameToSystemLocation(name);
    d->systemLocation = name;
}


/*!
    Returns the name set by setPort() or passed to the WSerialPort constructor.
    This name is short, i.e. it is extracted and converted from the internal
    variable system location of the device. The conversion algorithm is
    platform specific:
    \table
    \header
        \li Platform
        \li Brief Description
    \row
        \li Windows
        \li Removes the prefix "\\\\.\\" or "//./" from the system location
           and returns the remainder of the string.
    \row
        \li Unix, BSD
        \li Removes the prefix "/dev/" from the system location
           and returns the remainder of the string.
    \endtable

    \sa setPort(), WSerialPortInfo::portName()
*/
QString WSerialPort::portName() const   {
    Q_D(const WSerialPort);
    return d->systemLocation;
//    return WSerialPortPrivate::portNameFromSystemLocation(d->systemLocation);
}

/*!
    \reimp

    Opens the serial port using OpenMode \a mode, and then returns \c true if
    successful; otherwise returns \c false and sets an error code which can be
    obtained by calling the error() method.

    \note The method returns \c false if opening the port is successful, but could
    not set any of the port settings successfully. In that case, the port is
    closed automatically not to leave the port around with incorrect settings.

    \warning The \a mode has to be QIODevice::ReadOnly, QIODevice::WriteOnly,
    or QIODevice::ReadWrite. Other modes are unsupported.

    \sa QIODevice::OpenMode, setPort()
*/
bool WSerialPort::open(OpenMode mode)   {
    Q_D(WSerialPort);

    if (isOpen()) {
        d->setError(WSerialPortErrorInfo(WSerialPort::OpenError));
        return false;
    }
    // Define while not supported modes.
    static const OpenMode unsupportedModes = Append | Truncate | Text | Unbuffered;
    if ((mode & unsupportedModes) || mode == NotOpen) {
        d->setError(WSerialPortErrorInfo(WSerialPort::UnsupportedOperationError, tr("Unsupported open mode")));
        return false;
    }
    clearError();
    if (!d->open(mode))     return false;
    QIODevice::open(mode);
    return true;
}

/*!
    \reimp

    \note The serial port has to be open before trying to close it; otherwise
    sets the NotOpenError error code.

    \sa QIODevice::close()
*/
void WSerialPort::close()   {
    Q_D(WSerialPort);
    if (!isOpen()) {
        d->setError(WSerialPortErrorInfo(WSerialPort::NotOpenError));
        return;
    }
    d->close();
    QIODevice::close();
}

/*!
    This function writes as much as possible from the internal write
    buffer to the underlying serial port without blocking. If any data
    was written, this function returns \c true; otherwise returns \c false.

    Call this function for sending the buffered data immediately to the serial
    port. The number of bytes successfully written depends on the operating
    system. In most cases, this function does not need to be called, because the
    WSerialPort class will start sending data automatically once control is
    returned to the event loop. In the absence of an event loop, call
    waitForBytesWritten() instead.

    \note The serial port has to be open before trying to flush any buffered
    data; otherwise returns \c false and sets the NotOpenError error code.

    \sa write(), waitForBytesWritten()
*/
bool WSerialPort::flush()   {
    Q_D(WSerialPort);

    if (!isOpen()) {
        d->setError(WSerialPortErrorInfo(WSerialPort::NotOpenError));
        qWarning("%s: device not open", Q_FUNC_INFO);
        return false;
    }
    return d->flush();
}

/*!
    Discards all characters from the output or input buffer, depending on
    given directions \a directions. This includes clearing the internal class buffers and
    the UART (driver) buffers. Also terminate pending read or write operations.
    If successful, returns \c true; otherwise returns \c false.

    \note The serial port has to be open before trying to clear any buffered
    data; otherwise returns \c false and sets the NotOpenError error code.
*/
//bool WSerialPort::clear(Directions directions)  {
//    Q_D(WSerialPort);

//    if (!isOpen()) {
//        d->setError(WSerialPortErrorInfo(WSerialPort::NotOpenError));
//        qWarning("%s: device not open", Q_FUNC_INFO);
//        return false;
//    }
//    if (directions & Input)        d->buffer.clear();
//    if (directions & Output)       d->writeBuffer.clear();
//    return d->clear(directions);
//}

/*!
    \reimp

    Returns \c true if no more data is currently available for reading; otherwise
    returns \c false.

    This function is most commonly used when reading data from the
    serial port in a loop. For example:

    \code
    // This slot is connected to WSerialPort::readyRead()
    void WSerialPortClass::readyReadSlot()
    {
        while (!port.atEnd()) {
            QByteArray data = port.read(100);
            ....
        }
    }
    \endcode

     \sa bytesAvailable(), readyRead()
 */
bool WSerialPort::atEnd() const     {
    return QIODevice::atEnd();
}
/*!
    \property WSerialPort::error
    \brief the error status of the serial port

    The I/O device status returns an error code. For example, if open()
    returns \c false, or a read/write operation returns \c -1, this property can
    be used to figure out the reason why the operation failed.

    The error code is set to the default WSerialPort::NoError after a call to
    clearError()
*/
WSerialPort::SerialPortError WSerialPort::error() const {
    Q_D(const WSerialPort);
    return d->error;
}

void WSerialPort::clearError()  {
    Q_D(WSerialPort);
    d->setError(WSerialPortErrorInfo(WSerialPort::NoError));
}

/*!
    \fn void WSerialPort::error(SerialPortError error)
    \obsolete

    Use errorOccurred() instead.
*/

/*!
    \fn void WSerialPort::errorOccurred(SerialPortError error)
    \since 5.8

    This signal is emitted when an error occurs in the serial port.
    The specified \a error describes the type of error that occurred.

    \sa WSerialPort::error
*/

/*!
    Returns the size of the internal read buffer. This limits the
    amount of data that the client can receive before calling the read()
    or readAll() methods.

    A read buffer size of \c 0 (the default) means that the buffer has
    no size limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
*/
qint64 WSerialPort::readBufferSize() const  {
    Q_D(const WSerialPort);
    return d->readBufferMaxSize;
}

/*!
    Sets the size of WSerialPort's internal read buffer to be \a
    size bytes.

    If the buffer size is limited to a certain size, WSerialPort
    will not buffer more than this size of data. The special case of a buffer
    size of \c 0 means that the read buffer is unlimited and all
    incoming data is buffered. This is the default.

    This option is useful if the data is only read at certain points
    in time (for instance in a real-time streaming application) or if the serial
    port should be protected against receiving too much data, which may
    eventually cause the application to run out of memory.

    \sa readBufferSize(), read()
*/
void WSerialPort::setReadBufferSize(qint64 size)    {
    Q_D(WSerialPort);
    d->readBufferMaxSize = size;
    if (isReadable())   d->startAsyncRead();
}

/*!
    \reimp

    Always returns \c true. The serial port is a sequential device.
*/
bool WSerialPort::isSequential() const  {
    return true;
}

/*!
    \reimp

    Returns the number of incoming bytes that are waiting to be read.

    \sa bytesToWrite(), read()
*/
qint64 WSerialPort::bytesAvailable() const  {
    return QIODevice::bytesAvailable();
}

/*!
    \reimp

    Returns the number of bytes that are waiting to be written. The
    bytes are written when control goes back to the event loop or
    when flush() is called.

    \sa bytesAvailable(), flush()
*/
qint64 WSerialPort::bytesToWrite() const
{
    qint64 pendingBytes = QIODevice::bytesToWrite();
#if defined(Q_OS_WIN32)
    pendingBytes += d_func()->writeChunkBuffer.size();
#endif
    return pendingBytes;
}

/*!
    \reimp

    Returns \c true if a line of data can be read from the serial port;
    otherwise returns \c false.

    \sa readLine()
*/
bool WSerialPort::canReadLine() const   {
    return QIODevice::canReadLine();
}

/*!
    \reimp

    This function blocks until new data is available for reading and the
    \l{QIODevice::}{readyRead()} signal has been emitted. The function
    will timeout after \a msecs milliseconds; the default timeout is
    30000 milliseconds. If \a msecs is -1, this function will not time out.

    The function returns \c true if the readyRead() signal is emitted and
    there is new data available for reading; otherwise it returns \c false
    (if an error occurred or the operation timed out).

    \sa waitForBytesWritten()
*/
bool WSerialPort::waitForReadyRead(int msecs)   {
    Q_D(WSerialPort);
    return d->waitForReadyRead(msecs);
}

/*!
    \fn Handle WSerialPort::handle() const
    \since 5.2

    If the platform is supported and the serial port is open, returns the native
    serial port handle; otherwise returns \c -1.

    \warning This function is for expert use only; use it at your own risk.
    Furthermore, this function carries no compatibility promise between minor
    Qt releases.
*/

/*!
    \reimp

    This function blocks until at least one byte has been written to the serial
    port and the \l{QIODevice::}{bytesWritten()} signal has been emitted. The
    function will timeout after \a msecs milliseconds; the default timeout is
    30000 milliseconds. If \a msecs is -1, this function will not time out.

    The function returns \c true if the bytesWritten() signal is emitted; otherwise
    it returns \c false (if an error occurred or the operation timed out).
*/
bool WSerialPort::waitForBytesWritten(int msecs)    {
    Q_D(WSerialPort);
    return d->waitForBytesWritten(msecs);
}

/*!
    \reimp

    \omit
    This function does not really read anything, as we use QIODevicePrivate's
    buffer. The buffer will be read inside of QIODevice before this
    method will be called.
    \endomit
*/
qint64 WSerialPort::readData(char *data, qint64 maxSize)    {
    Q_UNUSED(data)
    Q_UNUSED(maxSize)
    // In any case we need to start the notifications if they were
    // disabled by the read handler. If enabled, next call does nothing.
    d_func()->startAsyncRead();
    // return 0 indicating there may be more data in the future
    return qint64(0);
}

/*!
    \reimp
*/
qint64 WSerialPort::readLineData(char *data, qint64 maxSize)    {
    return QIODevice::readLineData(data, maxSize);
}

/*!
    \reimp
*/
qint64 WSerialPort::writeData(const char *data, qint64 maxSize) {
    Q_D(WSerialPort);
    return d->writeData(data, maxSize);
}


#include "moc_WSerialPort.cpp"
