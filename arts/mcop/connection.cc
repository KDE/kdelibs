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

#include "connection.h"
#include "dispatcher.h"
#include <stdio.h>

Connection::Connection()
{
	_connState = unknown;
}

Connection::~Connection()
{
}

void Connection::initReceive()
{
	rcbuf = 0;
	receiveHeader = true;
	remaining = 12;
}

void Connection::receive(unsigned char *data, long len)
{
	if(len > remaining)
	{
		unsigned char *data2 = data+remaining;
		long len2 = len-remaining;
		receive(data,remaining);

		/* This could be optimized to a non recursive thing (fixme?) */
		receive(data2,len2);
		return;
	}
	// get a buffer for the incoming message:
	if(!rcbuf) rcbuf = new Buffer;

	remaining -= len;
	rcbuf->write(data,len);

#ifdef DEBUG_IO
	printf("read %ld bytes\n",len);
#endif

	if(remaining == 0)
	{
		if(receiveHeader)
		{
			long mcopMagic;

			mcopMagic = rcbuf->readLong();
			remaining = rcbuf->readLong() - 12;
			messageType = rcbuf->readLong();

			if(_connState != Connection::established && remaining >= 4096)
			{
				/*
				 * don't accept large amounts of data on unauthenticated
				 * connections
				 */
				remaining = 0;
			}

			if(mcopMagic == MCOP_MAGIC)
			{
				// do we need to receive more data (message body?)
				if(remaining)
				{
					receiveHeader = false;
				}
				else
				{
					Buffer *received = rcbuf;
					initReceive();
					Dispatcher::the()->handle(this,received,messageType);
				}
			}
			else
			{
				initReceive();
				Dispatcher::the()->handleCorrupt(this);
			}
		}
		else
		{
			Buffer *received = rcbuf;

			/*
			 * it's important to prepare to receive new messages *before*
			 * calling Dispatcher::the()->handle(...), as handle may
			 * get into an I/O situation (e.g. when doing an invocation
			 * itself), and we may receive more messages while handle is
			 * running
			 */
			initReceive();

			// rcbuf is consumed by the dispatcher
			Dispatcher::the()->handle(this,received,messageType);
		}
	}
}
