/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "connection.h"

#include <QtCore/QTimer>
#include <kde_file.h>	// KDE_fdopen

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <network/k3resolver.h>
#include <network/k3streamsocket.h>
#include <network/k3socketdevice.h>

#include <kdebug.h>
#include <QtCore/QSocketNotifier>

using namespace KIO;

class Connection::ConnectionPrivate
{
public:
    ConnectionPrivate()
        : fd_in( -1 )
        , f_out( 0 )
        , socket( 0 )
        , notifier( 0 )
        , receiver( 0 )
        , member( 0 )
        , m_suspended( false )
    {}
    int fd_in;
#ifdef Q_WS_WIN
    int f_out;
#else
    FILE *f_out;
#endif
    KNetwork::KStreamSocket *socket;
    QSocketNotifier *notifier;
    QObject *receiver;
    const char *member;
    QQueue<Task> m_tasks;
    bool m_suspended;
};

Connection::Connection()
    : d( new ConnectionPrivate )
{
}

Connection::~Connection()
{
    close();
    delete d;
}

void Connection::suspend()
{
    d->m_suspended = true;
    if (d->notifier)
       d->notifier->setEnabled(false);
    if (d->socket) {
        d->socket->enableRead(false);
    }
}

void Connection::resume()
{
    d->m_suspended = false;
    if (d->notifier)
       d->notifier->setEnabled(true);
    if (d->socket) {
        d->socket->enableRead(true);
    }
}

void Connection::close()
{
    delete d->notifier;
    d->notifier = 0;
    delete d->socket;
    d->socket = 0;

    // KSocket has already closed the file descriptor, but we need to
    // close the file-stream as well otherwise we leak memory.
    // As a result we close the file descriptor twice, but that should
    // be harmless
    // KDE4: fix this
#ifndef Q_WS_WIN
    if (d->f_out)
       fclose(d->f_out);
#endif
    d->f_out = 0;
    d->fd_in = -1;
    d->m_tasks.clear();
}

int Connection::fd_from() const
{
    return d->fd_in;
}

int Connection::fd_to() const
{
#ifdef Q_WS_WIN
    return d->f_out;
#else
    return fileno( d->f_out );
#endif
}

bool Connection::inited() const
{
    return (d->fd_in != -1) && (d->f_out != 0);
}

bool Connection::suspended() const
{
    return d->m_suspended;
}

bool Connection::send(int cmd, const QByteArray& data)
{
    if (!inited() || !d->m_tasks.isEmpty()) {
	Task task;
	task.cmd = cmd;
	task.data = data;
	d->m_tasks.enqueue(task);
        return true;
    } else {
	return sendnow( cmd, data );
    }
}

void Connection::dequeue()
{
    if (!inited())
	return;

    while (!d->m_tasks.isEmpty())
    {
       const Task task = d->m_tasks.dequeue();
       sendnow( task.cmd, task.data );
    }
}

void Connection::init(KNetwork::KStreamSocket *sock)
{
#ifdef Q_WS_WIN
    sock->setFamily(KNetwork::KResolver::InetFamily);
#else
    sock->setFamily(KNetwork::KResolver::LocalFamily);
#endif
    sock->setBlocking(true);
    sock->connect();

    delete d->notifier;
    d->notifier = 0;
    delete d->socket;
    d->socket = sock;
    d->fd_in = d->socket->socketDevice()->socket();
#ifdef Q_WS_WIN
    d->f_out = d->fd_in;
#else
    d->f_out = KDE_fdopen( d->socket->socketDevice()->socket(), "wb" );
#endif
    if (d->receiver && ( d->fd_in != -1 )) {
        d->notifier = 0L;
	if ( d->m_suspended ) {
            suspend();
	} else {
            QObject::connect(d->socket, SIGNAL(readyRead()), d->receiver, d->member);
	}
    }
    dequeue();
}

void Connection::init(int _fd_in, int fd_out)
{
    delete d->notifier;
    d->notifier = 0;
    d->fd_in = _fd_in;
#ifdef Q_WS_WIN
    d->f_out = fd_out;
#else
    d->f_out = KDE_fdopen( fd_out, "wb" );
#endif
    if (d->receiver && ( d->fd_in != -1 )) {
	d->notifier = new QSocketNotifier(d->fd_in, QSocketNotifier::Read, this);
	if ( d->m_suspended ) {
            suspend();
	}
	QObject::connect(d->notifier, SIGNAL(activated(int)), d->receiver, d->member);
    }
    dequeue();
}


void Connection::connect(QObject *_receiver, const char *_member)
{
    d->receiver = _receiver;
    d->member = _member;
    delete d->notifier;
    d->notifier = 0;
    if (d->receiver && (d->fd_in != -1 )) {
        if (d->socket == 0L)
	    d->notifier = new QSocketNotifier(d->fd_in, QSocketNotifier::Read, this);
        if ( d->m_suspended )
            suspend();

	if (d->notifier)
	    QObject::connect(d->notifier, SIGNAL(activated(int)), d->receiver, d->member);
	if (d->socket)
	    QObject::connect(d->socket, SIGNAL(readyRead()), d->receiver, d->member);
    }
}

bool Connection::sendnow( int _cmd, const QByteArray &data )
{
    if (d->f_out == 0) {
	return false;
    }

    if (data.size() > 0xffffff)
        return false;

    static char buffer[ 64 ];
    sprintf( buffer, "%6x_%2x_", data.size(), _cmd );
#ifdef Q_WS_WIN
    size_t n = ::send( d->f_out, buffer,10, 0 );
#else
    size_t n = fwrite( buffer, 1, 10, d->f_out );
#endif
    if ( n != 10 ) {
	kError(7017) << "Could not send header" << endl;
	return false;
    }

#ifdef Q_WS_WIN
    n = ::send( d->f_out, data.data(), data.size(), 0);
#else
    n = fwrite( data.data(), 1, data.size(), d->f_out );
#endif
    if ( n != (size_t)data.size() ) {
	kError(7017) << "Could not write data" << endl;
	return false;
    }

#ifndef Q_WS_WIN
    fflush( d->f_out );
#endif
    return true;
}

int Connection::read( int* _cmd, QByteArray &data )
{
    if (d->fd_in == -1 ) {
	kError(7017) << "read: not yet inited" << endl;
	return -1;
    }

    static char buffer[ 10 ];

 again1:
#ifdef Q_WS_WIN
    ssize_t n = ::recv( d->fd_in, buffer, 10, 0);
#else
    ssize_t n = ::read( d->fd_in, buffer, 10);
#endif
    if ( n == -1 && errno == EINTR )
	goto again1;
#ifdef Q_WS_WIN
	if ( n == -1 && WSAGetLastError() == WSAEWOULDBLOCK ) {
        kDebug(7017) << "Header read would block detected" << endl;
        return -2;
    }
#endif
    if ( n == -1) {
	kError(7017) << "Header read failed, errno=" << errno << endl;
    }

    if ( n != 10 ) {
      if ( n ) // 0 indicates end of file
        kError(7017) << "Header has invalid size (" << n << ")" << endl;
      return -1;
    }

    buffer[ 6 ] = 0;
    buffer[ 9 ] = 0;

    char *p = buffer;
    while( *p == ' ' ) p++;
    long int len = strtol( p, 0L, 16 );

    p = buffer + 7;
    while( *p == ' ' ) p++;
    long int cmd = strtol( p, 0L, 16 );

    data.resize( len );

    if ( len > 0L ) {
	size_t bytesToGo = len;
	size_t bytesRead = 0;
	do {
#ifdef Q_WS_WIN
      n = ::recv(d->fd_in, data.data()+bytesRead, bytesToGo, 0);
#else
	    n = ::read(d->fd_in, data.data()+bytesRead, bytesToGo);
#endif
	    if (n == -1) {
		if (errno == EINTR)
		    continue;
#ifdef Q_WS_WIN
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
            kDebug(7017) << "Would block detect" << endl;
            Sleep(20);
            continue;
        }
#endif
		kError(7017) << "Data read failed, errno=" << errno << endl;
		return -1;
	    }

	    bytesRead += n;
	    bytesToGo -= n;
	}
	while(bytesToGo);
    }

    *_cmd = cmd;
    return len;
}

#include "connection.moc"
