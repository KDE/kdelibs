/* This file is part of the KDE project
 *
 * Copyright (C) 2001 - 2003 Mirko Boehm <mirko@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.

 * This file declares an IO device using SSL connections.

 * $ Author: Mirko Boehm $
 * $ Copyright: (C) 2003, Mirko Boehm $
 * $ Contact: mirko@kde.org
 *            http://www.kde.org
 *            http://www.hackerbuero.org $

 *  $Id$
*/

#ifndef SSLIO_H
#define SSLIO_H

#include <qiodevice.h>
#include <qobject.h>

class QSocketNotifier;
class QMutex;
class QDns;

namespace KDESSL {

class IODevice : public QObject, public QIODevice
{
    Q_OBJECT

public:

    enum ErrorCode {
        /** The respective method is not implemented. */
        NotImplemented,
        /** The method succeeded. */
        Success,
        /** Same as Success, for better reading. */
        NoError = Success,
        /** The request was refused, think of a rejected login or so. */
        Refused,
        /** The request failed due to a protocol or network error, not due
            to a proper but rejected handshake, for example. */
        Failure,
        /** An operating system error (including file handling or networking errors)
            occured for a reason out of the control of this code
        */
        SysError,
        /** The request would result in blocking the calling thread but
            this was forbidden. */
        WouldBlock,
        /** There is no connection or it has been closed. Maybe you tried
            to read or write data but the other side already sent a
            close_notify and closed the connection afterwards (what is
            perfectly legal under the SSL/TLS specifications).
        */
        NoConnection,
        /** A buffer overflow occured. If you tried to read from the
            connection but the read buffer is filled completely with
            available data, this might happen, for example.
        */
        BufferOverflow,
        /** A connection was refused. */
        ConnectionRefused,
        /** The protocol used to connect (TCP) is not supported. */
        ProtocolNotSupported,
        /** The host was not found. */
        HostNotFound,
        /** A socket read failed. */
        SocketRead,
        /** An error occured that was most likely caused by a flaw in the
            own code (if that is decisable).
        */
        InternalError,
	NoSSL
    };

    enum Party {
        /** This is the server side. */
        Server,
        /** This is the client side. */
        Client
    };

    enum State {
        /** Not connected. */
        NotConnected = 0,
        Idle = NotConnected,
        /** Looking up the host to connect to. */
        HostLookup,
        /** Connecting to the host. */
        Connecting,
        /** Connection is established. */
        Connection,
        Connected = Connection,
        /** Closing down the connection. */
        Closing
    };

    /** Create an IO device that uses SSL over TCP sockets. The SSL
        context (SSL_CTX) must have been created before.
        Party defines wether this SSLIO object acts as a server or
        client part of the SSL protocol.
    */
    IODevice(void *ctx_, Party p=Client);

    virtual ~IODevice();

    /** Get the connection state. */
    State state();
    
    /** Establish a connection using the given file descriptor (that is
        supposed to be a TCP connection).
        @return Success Handshake ended successfully
        @return Refused Handshake ended properly, but without success
        @return Failure Handshake ended in failure
    */
    ErrorCode setFd(int fd);

    /** Return the file descriptor. */
    int fd() const;

    /** Connect to the host at the given port. */
    virtual void connectToHost(const QString& host, unsigned port);

    /** Set blocking read/write calls.
     */
    ErrorCode setBlocking(bool state);

    /** Return whether this IO device uses blocking socket calls or
        not. */
    bool blocking();
    
    // methods reimplemented from QIODevice
    bool open(int mode);
    
    void close();
    
    void flush();
    
    QIODevice::Offset size() const;

    Q_LONG readBlock(char* data, Q_ULONG maxlen);

    Q_LONG writeBlock(const char* data, Q_ULONG len);
    
    /** Get the available data until the next newline character. Please
        note that there has to be a newline, EOD is not sufficient. The
        newline is contained in the returned text (which ends in a
        binary 0 byte).
    */
    int readLine(char* data, uint maxlen);
    
    int getch();
    
    int putch(int);
    
    int ungetch(int); 
    
    /** Return the SSL context associated with this object.
     */
    void* sslCtx() const;
    
    /** Return the SSL object associated with this connection.
        You should know what you are doing when fiddling around with it.
     */
    void *ssl() const;
    
    /** Take over the connection and make sure the other IO device
        does not use it anymore. Use this method if you want to delete
        an object that bears a connection you want to handle in this
        object in future.
    */
    bool takeOver(IODevice* theother);
    
protected:
    /** Define if we act as client or server. The SSL protocol
        requires us to act differently depending on wether we
        initiated the connection (client) or another party is
        connecting to us (server).
    */
    const Party party;
    
    /** The file descriptor. Zero before used. */
    int m_fd;
    
    /** The socket notifier for reading. */
    QSocketNotifier *snRead;
    
    /** The socket notifier for writing. */
    QSocketNotifier *snWrite;
    
    // ----- members to implement the read buffer:
    /** The read buffer. We use a plain char array to be as performant
        as possible with the C style OpenSSL methods.
        Will be allocated in the constructor and freed in the
        destructor.
        Note that writes are not buffered.
    */
    char *rbuf;
    
    /** Size (in bytes) of the currently used read buffer.
     */
    int rbufSize;
    
    /** Bytes used (and available to the application) of the read
        buffer. rbufUsed+rbufOffset is the next available byte.
    */
    int rbufUsed;
    
    /** Current offset of the read buffer. The bytes from the beginning
        to the byte at [Offset] are no more valid. If the next SSL_read
        would write over the buffers end, the bytes available will be
        moved to the beginning of the buffer and the offset set to
        zero. */
    int rbufOffset;
    
    /** Clean up the space used in the read buffer offset. rbufOffset is
        zeroed afterwards.
    */
    void cleanupRBufOffset();
    
    /** Fill the read buffer with bytes from the SSL connection as far
        as possible. Whether this method blocks or not depends on the
        behavior set for your socket (@see setBlocking). If the socket
        is blocking, this blocks the calling thread/process.
        Remember that if this method returns BufferOverflow, this is
        most probably no error condition, but non-handled application
        data that blocks reading new data from the socket.
        If data is read from the socket successfully, the signal
        readyRead is emitted by this method.
        @return WouldBlock if no data is readable from the socket
        @return BufferOverflow if the read buffer is filled with data
        @return Success if new data is available afterwards
    */
    ErrorCode fillReadBuffer();
    
    // ----- members to implement the write buffer:
    /** The write buffer. */
    char* wbuf;
    
    /** The write buffer size. */
    int wbufSize;
    
    /** The bytes stored in the write buffer, */
    int wbufUsed;
    
    /** Bytes used at the beginning of the write buffer that are no more valid. */
    int wbufOffset;

    /** Reset the write buffers offset. */

    void cleanupWBufOffset();
    // ^^^^^ end of write buffer parts
 
    /** A method to clean up the given buffer. The idea of the buffers
        (read and write buffer, here) is to just move an offset when bytes
        are consumed and only moving the beginning of the valid bytes to
        the start of the buffer when an overflow occurs. This should
        minimize the memove operations, hopefully.
        Aware1: to keep thread safety, protect the buffer before
        calling the method!
        Aware2: set the offset to zero afterwards!
        Aware3: this method is protected for a reason!
        Aware4: buffer is deallocated!
        @returns The pointer to the buffer (update it, since it might
        have been reallocated)
    */
    char* cleanupBufOffset(char* buffer, int size, int used, int offset);

    /** The connection state. */
    State m_state;
    
    /** Used to do the host lookups. */
    QDns *dns;
    
    /** Mutex to access the device. */
    QMutex *mutex;
    
    /** The host we are supposed to connect to. */
    QString host;
    
    /** The port we are supposed to be connected to. */
    unsigned port;
    
    /** Set the connection state. */
    void setState(State);
    
signals:
    /** Connect to this signal to get notified on incoming data.
     */
    void readyRead(int socket);

    /** Connect to this signal to be notified when data has been sent
        and removed from the write buffer. */
    void bytesWritten(int);
    
    /** Connect to this signal to get notified when the connection is
        able to accept more bytes to write (the bytes might be buffered,
        though, before they are written to the actual network
        connection.
    */
    // void readyWrite(int socket);

    /** Connect to this signal to get notified when a number of bytes
        have been written.
    */
    // void bytesWritten(int socket, int noOfBytes);

    /** Connect to this signal to get notified on connections shut down
        by the other side.
        Aware: this does not mean the connection has been closed
        properly! It just means that the other side sent the
        SSL_shutdown request.
        Aware 2: there might still be data available in the read
        buffer!
        Aware 3: do not delete the object on shutdown, you are inside
        a method of this object!
    */
    void shutdown(IODevice*);

    /** Announce error conditions on the SSL connection. See @see
        ErrorCode for  the different values. */
    void error(ErrorCode);

    /** The host given to a connectToHost call was found. */
    void hostFound();

    /** The handshake ended successfully. */
    void connected();

    /** For finer grained control over the result of a call to
        connectToHost, connect to this signal and evaluate the
        errorcode.
        Possible values are:
        - HostNotFound if the DNS query did not succeed.
        - ConnectionRefused if the server did not accept the
        connection (might be a wrong IP or port, too).
        - ProtocolNotSupported if the socket could not be created.
        - Refused if the connection was closed after a SSL handshake
        that did not succeed.
        - Success if the connection has been created flawlessly.
        You could notice most of this information by watching m_state
        and listening to the error() signal, but here you get all in
        one signal and a message on success, too.
    */
    void connectResult(IODevice::ErrorCode);

    /** The connection on the socket used has been closed (after the
        SSL_shutdown has been performed. */
    void disconnected(IODevice*);

    /** Connection state changed. */
    void stateChanged(IODevice::State);

protected slots:
    /** Catch signals emitted by the read notifier.  */
    virtual void slotReadNotification(int socket);

    /** Catch signals emitted by the write notifier. */
    virtual void slotWriteNotification(int socket);

    /** Try to establish a connection. */
    void tryConnecting();
    
    /** Try to write data to the write buffer. */
    virtual void writeToSocket();

private:
    class Private;
    Private *d;
};

}

#endif // defined SSLIO_H
