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

 * This file implements an IO device using SSL connections.

 * $ Author: Mirko Boehm $
 * $ Copyright: (C) 2003, Mirko Boehm $
 * $ Contact: mirko@kde.org
 *            http://www.kde.org
 *            http://www.hackerbuero.org $

 * $Id$
*/

#include "sslio.h"
// #include "qdaemon.h"

#include <qsocketnotifier.h>
#include <qmutex.h>
#include <qdns.h>

extern "C" {
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
};

using namespace KDESSL;

const int RBufDefaultSize = 16*1024;
const int WBufDefaultSize = RBufDefaultSize;
/** We do not set the buffers to start at zero because of the
    following effect:
    If some data arrives and fills the buffer so that is is rearranged
    using cleanupBufferOffset, and some code tries a unget(..), the
    whole buffer has to be moved again. Since this does not happen
    often, a few bytes are enough.
    See how QTextStream looks two bytes ahead, for example, to see why
    this is necessary.
*/
const int DefaultBufferOffset = 8;

IODevice::IODevice(SSL_CTX *ctx_, Party p)
    : QIODevice(),
      party(p),
      m_fd(0),
      ctx(ctx_),
      m_ssl(0),
      snRead(0),
      snWrite(0),
      rbuf(new char[RBufDefaultSize]),
      rbufSize(RBufDefaultSize),
      rbufUsed(0),
      rbufOffset(DefaultBufferOffset),
      wbuf(new char[WBufDefaultSize]),
      wbufSize(WBufDefaultSize),
      wbufUsed(0),
      wbufOffset(DefaultBufferOffset),
      m_state(Idle),
      dns(0),
      mutex(new QMutex(true))
{
}

IODevice::~IODevice()
{
    mutex->lock();
    if(m_fd!=0) close();
    if(rbuf!=0) { delete rbuf; rbuf=0; }
    mutex->unlock();
    delete mutex;
}

IODevice::ErrorCode IODevice::setFd(int fd_)
{
    int err;
    IODevice::ErrorCode rc;
    // -----
    mutex->lock();
    m_ssl=SSL_new(ctx);
    if(m_ssl==0)
    {
        rc=Failure;
    } else {
        SSL_set_fd(m_ssl, fd_);
        if(party==Server)
        {
            err=SSL_accept(m_ssl);
        } else {
            err=SSL_connect(m_ssl);
        }
        // -----
        switch(err)
        {
        case 1: // successfully completed
            m_fd=fd_;
            // ----- make the fd non-blocking:
            // setBlocking(false); // not by default
            snRead=new QSocketNotifier(m_fd, QSocketNotifier::Read,
                                       this, "ReadNotifier");
            connect(snRead, SIGNAL(activated(int)),
                    SLOT(slotReadNotification(int)));
            snWrite=new QSocketNotifier(m_fd, QSocketNotifier::Write,
                                        this, "WriteNotifier");
            connect(snWrite, SIGNAL(activated(int)),
                    SLOT(slotWriteNotification(int)));
            setState(Connection);
            rc=Success;
            break;
        case 0: // handshake ended properly but was not successful
            ::close(fd_);
        rc=Refused;
        break;
        default: // a real error occured, most probably an OpenSSL error
            ::close(fd_);
        rc=Failure;
        };
    }
    mutex->unlock();
    return rc;
}

IODevice::ErrorCode IODevice::setBlocking(bool yes)
{
    int state;
    IODevice::ErrorCode rc;
    // -----
    mutex->lock();
    if((state=::fcntl(m_fd, F_GETFL, 0)))
    {
        if(::fcntl( m_fd, F_SETFL, yes ? (state&~O_NDELAY) : (state|O_NDELAY))<0)
	{
            switch(errno)
	    {
	    case EACCES:
	    case EBADF:
                rc=InternalError;
                break;
	    case EFAULT:
	    case EAGAIN:
#if EAGAIN != EWOULDBLOCK
	    case EWOULDBLOCK:
#endif
	    case EDEADLK:
	    case EINTR:
	    case EINVAL:
	    case EMFILE:
	    case ENOLCK:
	    case EPERM:
	    default:
                rc=SysError;
	    }
	} else {
            rc=Success;
	}
    } else {
        // our own FD is invalid?
        rc=InternalError;
    }
    mutex->unlock();
    return rc;
}

bool IODevice::blocking()
{
    bool rc;
    mutex->lock();
    int state=::fcntl(m_fd, F_GETFL, 0);
    if(state<0)
    {
        rc=(state & O_NONBLOCK)==0;
    } else {
        rc=false; // does not mean anything
    }
    mutex->unlock();
    return rc;
}

void IODevice::close()
{
    // WORK_TO_DO: handle close when there is data to be written

    // -----
    mutex->lock();
    if(m_state!=Idle)
    {
        if(snRead!=0)
        {
            delete snRead; snRead=0;
        }
        if(snWrite!=0)
        {        delete snWrite; snWrite=0;
        }
        SSL_shutdown(m_ssl);
        SSL_free(m_ssl); m_ssl=0;
        if(m_fd!=0)
        {
            ::close(m_fd);
            m_fd=0;
        }
        rbufUsed=0;
        rbufOffset=0;
        wbufUsed=0;
        wbufOffset=0;
    }
    mutex->unlock();
    if(m_state!=Idle)
    {
        setState(Idle);
        emit(disconnected(this));
    }
}

bool IODevice::open(int m)
{
    bool rc;
    mutex->lock();
    if(isOpen())
    {
        rc=false;
    } else {
        QIODevice::setMode(m & IO_ReadWrite);
        QIODevice::setState(IO_Open);
        rc=true;
    }
    mutex->unlock();
    return rc;
}

void IODevice::flush()
{
    writeToSocket();
}

QIODevice::Offset IODevice::size() const
{ // we do not read first since the application is expected to fill
    // the read buffer on read notifications
    QIODevice::Offset temp;
    mutex->lock();
    temp=rbufUsed;
    mutex->unlock();
    return temp;
}

Q_LONG IODevice::readBlock(char *data, Q_ULONG maxlen)
{
    Q_LONG rc;
    unsigned num;
    // unfortunately, we do have to copy the data from our read buffer
    // to the data buffer when using this method
    mutex->lock();
    // -----
    if(rbufUsed>0)
    { // there is data available
        num=QMIN(maxlen, (unsigned)rbufUsed);
        memcpy(data, rbuf+rbufOffset, num);
        rbufUsed-=num;
        rbufOffset+=num;
        rc=num;
    } else {
        rc=-1;
    }
    // -----
    mutex->unlock();
    return rc;
}

Q_LONG IODevice::writeBlock(const char* data, Q_ULONG len)
{
    Q_LONG rc;
    unsigned num;
    // ----- add data to write buffer:
    mutex->lock();
    if(len>(unsigned)wbufSize-wbufOffset-wbufUsed && wbufOffset>0)
    {
        // (this means a full memcpy of the bytes in data, but hey -
        // life's a bitch)
        cleanupWBufOffset();
    }
    if(wbufOffset+wbufUsed<wbufSize)
    { // there is space left in the write buffer
        num=QMIN(len, (unsigned)wbufSize-wbufOffset-wbufUsed);
        memcpy(wbuf+wbufUsed+wbufOffset, data, num);
        wbufUsed+=num;
        rc=num;
    } else {
        rc=-1;
    }
    // -----
    mutex->unlock();
    // ----- now trigger an event to write the data:
    snWrite->setEnabled(true);
    return rc;
}

void IODevice::writeToSocket()
{ // try to write the data available in the write buffer
    int sslerror, rc=0;
    bool syserror=false;
    snWrite->setEnabled(false);
    mutex->lock();
    // -----
    // make sure we ignore write events that do not handle user data
    // (remember SSL knows about other data types than user)
    if(wbufUsed!=0)
    {
        rc=SSL_write(m_ssl, wbuf+wbufOffset, wbufUsed);
        sslerror=SSL_get_error(m_ssl, rc);
        switch(sslerror)
        {
        case SSL_ERROR_NONE:
            // we expect rc to be leq than wbufUsed
            wbufOffset+=rc;
            wbufUsed-=rc;
            if(wbufUsed==0)
            { // the whole buffer has been written:
                wbufOffset=0;
            }
            break;
        case SSL_ERROR_WANT_WRITE:
            snWrite->setEnabled(true);
            // It is important not to increment wbufOffset here - OpenSSL
            // expects us to use the same buffer start address at the next
            // call to SSL_write!
            rc=0;
            break;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_X509_LOOKUP:
        case SSL_ERROR_WANT_CONNECT:
        case SSL_ERROR_SYSCALL:
        case SSL_ERROR_SSL:
        default:
            syserror=true;
        };
    }
    mutex->unlock();
    if(syserror) emit(error(SysError));
    if(rc>0) emit(bytesWritten(rc));
}

int IODevice::getch()
{
    int rc;
    char r;
    // -----
    mutex->lock();
    // -----
    if(rbufUsed>0)
    {
        r=rbuf[rbufOffset++];
        rbufUsed-=1;
        rc=r;
    } else {
        rc=-1;
    }
    // -----
    mutex->unlock();
    return rc;
}

int IODevice::putch(int c)
{ // no need to be thread safe here, writeBlock(..) is
    char data[2];
    data[0]=c; data[1]=0;
    return writeBlock(data, 2);
}

int IODevice::ungetch(int i)
{
    char c=(char)i;
    mutex->lock();
    if(rbufOffset==0)
    {
        // this one is VERY inefficient - to bad QTextStream does a
        // lookup with a unget(..) for the first two chars (the logic
        // of the read buffer does not allow rbufOffset to be zero,
        // in fact, this if(..) is academic) :
        cleanupRBufOffset();
    }
    rbuf[--rbufOffset]=c;
    ++rbufUsed;
    mutex->unlock();
    return i;
}

int IODevice::readLine(char *data, uint len)
{
    unsigned nl=0;
    int count, rc;
    // -----
    mutex->lock();
    // -----
    if(rbufUsed==0)
    {
        rc=-1; // no data available
    } else {
        for(count=rbufOffset; count<rbufOffset+rbufUsed; ++count)
	{
            if(rbuf[count]=='\n') break;
            ++nl;
	}
        // now nl contains the number of bytes until nl is encountered
        if(count!=rbufOffset+rbufUsed)
	{
            if(nl>=len-1)
	    { // not enough space in data available
                rc=-1;
	    } else {
                nl++; // include the newline
                memcpy(data, rbuf+rbufOffset, nl);
                data[nl]=0;
                rbufUsed-=nl;
                rbufOffset+=nl;
                rc=nl;
	    }
	} else {
            // no newline found
            rc=-1;
	}
    }
    mutex->unlock();
    return rc;
}

void IODevice::slotReadNotification(int /* socket */)
{
    fillReadBuffer();
}

void IODevice::slotWriteNotification(int /* socket */)
{
    writeToSocket();
}

IODevice::ErrorCode IODevice::fillReadBuffer()
{
    int r;
    char* data; // current data pointer
    int len; // bytes remaining available in the read buffer
    ErrorCode rc;
    // -----
    mutex->lock();
    // -----
    do
    {
        if(rbufOffset+rbufUsed>=rbufSize)
	{
            if(rbufOffset>DefaultBufferOffset)
	    {
                cleanupRBufOffset();
	    } else {
                rc=BufferOverflow;
                break;
	    }
	}
        len=rbufSize-rbufOffset-rbufUsed;
        data=rbuf+rbufOffset+rbufUsed;
        r=SSL_read(m_ssl, data, len);
        switch(SSL_get_error(m_ssl, r))
	{
	case SSL_ERROR_NONE:
            rbufUsed+=r; // offset remains unchanged here
            break;
	case SSL_ERROR_ZERO_RETURN:
            // this indicates a shutdown of the connection on the socket
            snRead->setEnabled(false);
            snWrite->setEnabled(false);
            // we are done reading:
            mutex->unlock();
            emit(shutdown(this));
            close();
            rc=NoConnection;
            goto leave;
            break;
	case SSL_ERROR_WANT_READ:
            break;
	default:
            break;
	};
    } while(SSL_pending(m_ssl) && rbufOffset+rbufUsed<rbufSize);
    if(rbufUsed>0)
    {
        rc=NoError;
    } else {
        rc=WouldBlock;
    }
    mutex->unlock();
 leave:
    if(rbufUsed>0) emit(readyRead(m_fd));
    // if(rc==NoConnection) emit(shutdown(this));
    return rc;
}

// most of the code in connectToHost and tryConnecting is a 1-on-1
// copy of qsocket.cpp:

void IODevice::connectToHost(const QString& host_, unsigned port_)
{
    mutex->lock();
    // ----- immidiately close any open connection:
    if(m_state==Connected)
    {
        close();
    }
    // ----- look up the host name:
    //       cleanup dns first:
    if(dns!=0)
    {
        delete dns;
        dns=0;
    }
    //       fire up the connection process:
    setState(HostLookup);
    host=host_;
    port=port_;
    dns=new QDns(host, QDns::A);
    tryConnecting();
    if(m_state==HostLookup)
    {
        connect(dns, SIGNAL(resultsReady()), SLOT(tryConnecting()));
    }
    mutex->unlock();
}

void IODevice::tryConnecting()
{
    static QValueList<QHostAddress> l;
    int err;
    ErrorCode ec;
    // -----
    mutex->lock();
    if(m_state==HostLookup)
    { // dns is ready now:
        l=dns->addresses();
        if(l.isEmpty())
        {
            if(!dns->isWorking())
            {
                setState(Idle);
                emit(connectResult(HostNotFound));
                emit(error(HostNotFound));
            }
            mutex->unlock();
            return;
        }
        emit(hostFound());
        setState(Connecting);
    }
    if(m_state==Connecting)
    {
        int sd;
        struct sockaddr_in sa;
        sd =socket(AF_INET, SOCK_STREAM, 0);
        if(sd==-1)
        {
            switch(errno)
            {
            case EINVAL:
            case EPROTONOSUPPORT:
                emit(error(ProtocolNotSupported));
                break;
            case ENFILE:
            case EMFILE:
            case ENOBUFS:
            case ENOMEM:
                emit(error(InternalError));
                break;
            default:
                emit(error(InternalError));
            };
            emit(connectResult(ProtocolNotSupported));
            setState(Idle);
            mutex->unlock();
            return;
        }
        memset (&sa, '\0', sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_addr.s_addr = inet_addr (l[0].toString().ascii()); // Server IP
        sa.sin_port        = htons     (port); // Server Port number
        err=::connect(sd, (struct sockaddr*) &sa,
                      sizeof(sa));
        if(err==-1)
        {
            switch(errno)
            {
            case EBADF:
            case EFAULT:
            case ENOTSOCK:
            case EADDRINUSE:
            case EISCONN: // these are all considered flaws in this code:
                emit(error(InternalError));
                break;
            case ECONNREFUSED:
            case ETIMEDOUT:
                emit(error(ConnectionRefused));
                break;
            case ENETUNREACH:
            case EAGAIN:
                emit(error(SysError));
                break;
            case EAFNOSUPPORT:
                emit(error(ProtocolNotSupported));
                break;
            case EACCES:
            case EPERM:
                emit(error(Refused));
                break;
            };
            mutex->unlock();
            emit(connectResult(ConnectionRefused));
            setState(Idle);
            return;
        }
        ec=setFd(sd);
        if(ec!=Success)
        {
            switch(ec)
            {
            case Refused:
                emit(error(ConnectionRefused));
                break;
            case Failure:
                emit(error(Failure));
                break;
            default:
                emit(error(InternalError));
            };
            setState(Idle);
            emit(connectResult(Refused));
        } else {
            emit(connectResult(Success));
        }
    }
    mutex->unlock();
}

IODevice::State IODevice::state()
{
    IODevice::State rc;
    mutex->lock();
    rc=m_state;
    mutex->unlock();
    return rc;
}

void IODevice::setState(State s)
{
    bool change=false;
    mutex->lock();
    if(m_state!=s)
    {
        m_state=s;
        change=true;
    }
    mutex->unlock();
    if(change) emit(stateChanged(s));
}

void IODevice::cleanupRBufOffset()
{
    mutex->lock();
    rbuf=cleanupBufOffset(rbuf, rbufSize, rbufUsed, rbufOffset);
    rbufOffset=DefaultBufferOffset;
    mutex->unlock();
}

void IODevice::cleanupWBufOffset()
{
    mutex->lock();
    wbuf=cleanupBufOffset(wbuf, wbufSize, wbufUsed, wbufOffset);
    wbufOffset=DefaultBufferOffset;
    mutex->unlock();
}

char* IODevice::cleanupBufOffset(char* buffer, int size, int used, int offset)
{
    char *buf=new char[size];
    // all buffers begin with an offset of DefaultBufferOffset, so
    // there will never be more than size-DefaultBufferOffset bytes in
    // the buffer:
    memcpy(buf+DefaultBufferOffset, buffer+offset, used);
    delete buffer;
    return buf;
}

int IODevice::fd()
{
    return m_fd;
}

SSL_CTX *IODevice::sslCtx() const
{
    return ctx;
}

SSL *IODevice::ssl() const
{
    return m_ssl;
}

bool IODevice::takeOver(IODevice *other)
{
    bool rnEnable, wnEnable, rc=true;
    mutex->lock();
    other->mutex->lock();
    if(other->m_state!=Connected)
    {
        rc=false;
    } else {
        // ----- we locked both devices, so lets copy all essential
        // connection settings:
        m_fd=other->m_fd;
        ctx=other->ctx;
        m_ssl=other->m_ssl;
        rnEnable=other->snRead->isEnabled();
        wnEnable=other->snWrite->isEnabled();
        delete other->snRead; other->snRead=0;
        delete other->snWrite; other->snWrite=0;
        snRead=new QSocketNotifier(m_fd, QSocketNotifier::Read,
                                   this, "ReadNotifier");
        connect(snRead, SIGNAL(activated(int)),
                SLOT(slotReadNotification(int)));
        snWrite=new QSocketNotifier(m_fd, QSocketNotifier::Write,
                                    this, "WriteNotifier");
        connect(snWrite, SIGNAL(activated(int)),
                SLOT(slotWriteNotification(int)));
        snRead->setEnabled(rnEnable);
        snWrite->setEnabled(wnEnable);
        rbuf=other->rbuf;
        rbufSize=other->rbufSize;
        rbufUsed=other->rbufUsed;
        rbufOffset=other->rbufOffset;
        wbuf=other->wbuf;
        wbufSize=other->wbufSize;
        wbufUsed=other->wbufUsed;
        wbufOffset=other->wbufOffset;
        m_state=other->m_state;
        host=other->host;
        port=other->port;
        // ----- now set the other device to think it is not connected to
        // anybody:
        other->m_fd=0;
        other->setState(Idle);
        other->rbuf=0;
        other->rbufOffset=0;
        other->rbufUsed=0;
        other->wbuf=0;
        other->wbufOffset=0;
        other->wbufUsed=0;
    }
    other->mutex->unlock();
    mutex->unlock();
    return rc;
}

#include "sslio.moc"
