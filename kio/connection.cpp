// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ksock.h>
#include <qtimer.h>

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

#include "kio/connection.h"

#include <kdebug.h>

using namespace KIO;

template class QList<Task>;


Connection::Connection()
{
    f_out = 0;
    fd_in = 0;
    socket = 0;
    notifier = 0;
    receiver = 0;
    member = 0;
}

Connection::~Connection()
{
    close();
}

void Connection::suspend()
{
    if (notifier)
       notifier->setEnabled(false);
}

void Connection::resume()
{
    if (notifier)
       notifier->setEnabled(true);
}

void Connection::close()
{
    delete notifier;
    notifier = 0;
    delete socket;
    socket = 0;
    if (f_out)
       fclose(f_out);
    f_out = 0;
    fd_in = 0;
}

void Connection::send(int cmd, const QByteArray& data)
{
    if (!inited() || tasks.count() > 0) {
	kDebugInfo( 7007, "pending queue %d", cmd);
	Task *task = new Task();
	task->cmd = cmd;
	task->data = data;
	tasks.append(task);
    } else {
	sendnow( cmd, data );
    }

    if (tasks.count() > 0)
	QTimer::singleShot(100, this, SLOT(dequeue()));
}

void Connection::dequeue()
{
    if (tasks.count() == 0)
	return;

    kDebugInfo(7017, "dequeue");

    tasks.first();
    Task *task = tasks.take();
    sendnow( task->cmd, task->data );
    delete task;

    if (tasks.count() > 0)
	QTimer::singleShot(100, this, SLOT(dequeue()));
}


void Connection::init(int _fd_in, int fd_out)
{
    fd_in = _fd_in;
    f_out = fdopen( fd_out, "wb" );
}

void Connection::init(KSocket *sock)
{
    delete notifier;
    notifier = 0;
    delete socket;
    socket = sock;
    fd_in = socket->socket();
    f_out = fdopen( socket->socket(), "wb" );
    if (receiver && fd_in) {
	notifier = new QSocketNotifier(fd_in, QSocketNotifier::Read);
	QObject::connect(notifier, SIGNAL(activated(int)), receiver, member);
    }
}

void Connection::connect(QObject *_receiver, const char *_member)
{
    receiver = _receiver;
    member = _member;
    delete notifier;
    notifier = 0;
    if (receiver && fd_in) {
	notifier = new QSocketNotifier(fd_in, QSocketNotifier::Read);
	QObject::connect(notifier, SIGNAL(activated(int)), receiver, member);
    }
}

bool Connection::sendnow( int _cmd, const QByteArray &data )
{
    if (f_out == 0) {
	kDebugInfo(7017, "write: not yet inited.");
	return false;
    }

    static char buffer[ 64 ];
    sprintf( buffer, "%6x_%2x_", data.size(), _cmd );

    size_t n = fwrite( buffer, 1, 10, f_out );

    if ( n != 10 ) {
	kDebugError( 7017, "Could not send header");
	return false;
    }

    n = fwrite( data.data(), 1, data.size(), f_out );

    if ( n != data.size() ) {
	kDebugError( 7017, "Could not write data");
	return false;
    }

    fflush( f_out );

    return true;
}

int Connection::read( int* _cmd, QByteArray &data )
{
    if (!fd_in) {
	kDebugInfo(7017, "read: not yet inited");
	return -1;
    }

    static char buffer[ 10 ];

 again1:
    ssize_t n = ::read( fd_in, buffer, 10);
    if ( n == -1 && errno == EINTR )
	goto again1;

    if ( n == -1) {
	kDebugError( 7017, "Header read failed, errno=%d", errno);
    }

    if ( n != 10 ) {
      if ( n ) // 0 indicates end of file
        kDebugError( 7017, "Header has invalid size (%d)", n);
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
	int bytesToGo = len;
	int bytesRead = 0;
	do {
	    n = ::read(fd_in, data.data()+bytesRead, bytesToGo);
	    if (n == -1) {
		if (errno == EINTR)
		    continue;
		
		kDebugError( 7017, "Data read failed, errno=%d", errno);
		return -1;
	    }
	    if (n != bytesToGo) {
		kDebugInfo( 7017, "Not enough data read (%d instead of %d) cmd=%ld", n, bytesToGo, cmd);
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
