    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "tcpconnection.h"
#include "dispatcher.h"
#include "debug.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#define queue cqueue
#include <netdb.h>
#undef queue
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>

#undef DEBUG_CONNECTION_DATA

using namespace Arts;

/*
 * These parameters impact the performance significantly. There are two sides:
 *
 * when you use too large buffers for reading/writing
 *  - it may be, that each operation takes too long, so that for instance
 *    an important real time audio job drops during that time
 *  - it may be, that dealing with large buffers (allocation, cache, etc)
 *    itself causes a penalty
 *
 * on the other hand, small buffers lead to repeated operating system calls,
 * which is bad especially if you even go through all the chain: check what
 * can be written with select, invoke the corresponding object, actually
 * read (or write), do messages, do timers, go sleep again on select.
 */
static const int MCOP_MAX_READ_SIZE=8192;
static const int MCOP_MAX_WRITE_SIZE=8192;

SocketConnection::SocketConnection()
{
}

SocketConnection::SocketConnection(int _fd)
  : fd(_fd), _broken(false)
{
	arts_debug("socketconnection created, fd = %d",fd);
	Dispatcher::the()->ioManager()->watchFD(fd,
									IOType::read|IOType::except|IOType::reentrant,this);
	initReceive();
}

void SocketConnection::qSendBuffer(Buffer *buffer)
{
	if(_broken)
	{
		// forget it ;) - no connection there any longer
		delete buffer;
		return;
	}
	if(pending.size() == 0)
	{
		// if there is nothing pending already, it may be that we are lucky
		// and can write the buffer right now without blocking

		writeBuffer(buffer);
		if(!buffer->remaining())
		{
			delete buffer;
			return;
		}

		// but if it blocks, we'll need to watch for write chances to send
		// that buffer later
		Dispatcher::the()->ioManager()->watchFD(fd,IOType::write|IOType::reentrant,this);
	}
	pending.push_back(buffer);
}

#ifdef DEBUG_CONNECTION_DATA
static void connection_hex_dump(unsigned char *buffer, long len)
{
	int i = 0;
	printf("Connection: received %ld bytes:\n\n",len);
	while (i < len)
	{
		unsigned int j, n, d[16];

		for (n = 0; n < 16; n++)
			if (i < len)
		    	d[n] = buffer[i++];
		    else
		    	break;
		printf (" ");

		for (j = 0; j < n; j++) printf ("%s %02x", j == 8 ? "  " : "", d[j]);
		for (; j < 16; j++)		printf ("%s   ",   j == 8 ? "  " : "");
		printf ("    ");

		for (j = 0; j < n; j++)
			printf ("%c", d[j] >= 32 ? d[j] : '.');
		printf ("\n");
	}
}
#endif

void SocketConnection::notifyIO(int _fd, int types)
{
	assert(_fd == fd);

	if(types & IOType::read)
	{
		unsigned char buffer[MCOP_MAX_READ_SIZE];
		long n = read(fd,buffer,MCOP_MAX_READ_SIZE);

#ifdef DEBUG_CONNECTION_DATA
		connection_hex_dump(buffer,n);
#endif
	
		if(n > 0)
		{
			receive(buffer,n);
			// warning: the object may not exist any more here!
			return;
		}
		else if(n == 0 && errno != EAGAIN)
		{
			close(fd);
			_broken = true;
			Dispatcher::the()->ioManager()->remove(this,IOType::all);

			Dispatcher::the()->handleConnectionClose(this);
			// warning: the object may not exist any more here!
			return;
		}
	}

	if(types & IOType::write)
	{
		assert(pending.size() != 0);

		Buffer *pbuffer = *pending.begin();
		if(pbuffer->remaining()) writeBuffer(pbuffer);

		// no else => it could have been sent in the if(..remaining..) above
		if(!pbuffer->remaining())
		{
			delete pbuffer;
			pending.pop_front();

			if(pending.size() == 0)
				Dispatcher::the()->ioManager()->remove(this,IOType::write);
		}
	}

	if(types & IOType::except)
	{
		assert(false);
	}
}

void SocketConnection::writeBuffer(Buffer *buffer)
{
	long len = MCOP_MAX_WRITE_SIZE;
	if(buffer->remaining() < len) len = buffer->remaining();

	void *data = buffer->peek(len);
	long written = write(fd,data,len);

	if(written > 0)
		buffer->skip(len);
}

bool SocketConnection::broken()
{
	return _broken;
}

void SocketConnection::drop()
{
	if(!_broken)
	{
		close(fd);
		_broken = true;
		Dispatcher::the()->ioManager()->remove(this,IOType::all);
	}
}
