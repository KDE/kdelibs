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

#include "connection.h"
#include "dispatcher.h"
#include <stdio.h>

using namespace Arts;

Connection::Connection() :_refCnt(1)
{
	_connState = unknown;
}

Connection::~Connection()
{
	assert(_refCnt == 0);
}

void Connection::_copy()
{
	_refCnt++;
}

void Connection::_release()
{
	assert(_refCnt > 0);
	_refCnt--;
	if(_refCnt == 0)
		delete this;
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
	arts_debug("read %ld bytes",len);
#endif

	if(remaining == 0)
	{
		if(receiveHeader)
		{
			long mcopMagic;

			mcopMagic = rcbuf->readLong();
			remaining = rcbuf->readLong() - 12;
			messageType = rcbuf->readLong();

			if(_connState != Connection::established
			&& (remaining >= 4096 || remaining < 0))
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
