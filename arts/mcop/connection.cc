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

    */

#include "connection.h"
#include "dispatcher.h"
#include <stdio.h>

Connection::Connection()
{
	_ready = false;
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

			if(mcopMagic == MCOP_MAGIC)
			{
				// do we need to receive more data (message body?)
				if(remaining)
				{
					receiveHeader = false;
				}
				else
				{
					Dispatcher::the()->handle(this,rcbuf,messageType);
					initReceive();
				}
			}
			else
			{
				fprintf(stderr,"warning: got corrupt MCOP message!\n");
				initReceive();
			}
		}
		else
		{
			// rcbuf is consumed by the dispatcher
			Dispatcher::the()->handle(this,rcbuf,messageType);

			// prepare for the next message
			initReceive();
		}
	}
}
