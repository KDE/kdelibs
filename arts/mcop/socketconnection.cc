    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "tcpconnection.h"
#include "dispatcher.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#define queue cqueue
#include <netdb.h>
#undef queue
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>

SocketConnection::SocketConnection()
{
}

SocketConnection::SocketConnection(int fd)
{
	_broken = false;

	printf("socketconnection created, fd = %d\n",fd);
	this->fd = fd;
	Dispatcher::the()->ioManager()->watchFD(fd,
									IOType::read|IOType::except,this);
	initReceive();
}

void SocketConnection::qSendBuffer(Buffer *buffer)
{
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
		Dispatcher::the()->ioManager()->watchFD(fd,IOType::write,this);
	}
	pending.push_back(buffer);
}

void SocketConnection::notifyIO(int fd, int types)
{
	assert(fd == this->fd);

	if(types & IOType::read)
	{
		//printf("processing read notification\n");
		unsigned char buffer[1024];

		long n = read(fd,buffer,1024);
		//printf("ok, got %ld bytes\n",n);

		if(n > 0)
		{
			receive(buffer,n);
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
	long len = 1024;
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
