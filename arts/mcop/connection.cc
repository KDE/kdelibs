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
#include "debug.h"
#include <stdio.h>
#include <queue>
#include <algorithm>

using namespace Arts;
using namespace std;

namespace Arts {
class ConnectionPrivate {
public:
	struct Data {
        Data() : data(0), len(0) { }
		Data(unsigned char *data, long len) : data(data), len(len) { }
		Data(const Data& d) : data(d.data), len(d.len) { }
		unsigned char *data;
		long len;
	};

	queue<Data> incoming;
	map<string,string> hints;
};
};

Connection::Connection() :d(new ConnectionPrivate), _refCnt(1)
{
	_connState = unknown;
}

Connection::~Connection()
{
	assert(d->incoming.empty());
	assert(_refCnt == 0);

	delete d;
}

void Connection::_copy()
{
	assert(_refCnt > 0);
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

void Connection::receive(unsigned char *newdata, long newlen)
{
	/*
	 * protect against being freed while receive is running, as there are a
	 * few points where reentrant event loops may happen (Dispatcher::handle)
	 */
	_copy();

	d->incoming.push(ConnectionPrivate::Data(newdata,newlen));

	do
	{
		ConnectionPrivate::Data &data = d->incoming.front();

		// get a buffer for the incoming message
		if(!rcbuf) rcbuf = new Buffer;

		// put a suitable amount of input data into the receive buffer
		long len = min(remaining, data.len);

		remaining -= len;
		rcbuf->write(data.data,len);

		data.len -= len;
		data.data += len;

		if(data.len == 0)
			d->incoming.pop();

		// look if it was enough to do something useful with it
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
	} while(!d->incoming.empty());

	_release();
}

void Connection::setHints(const vector<string>& hints)
{
	vector<string>::const_iterator i;

	for(i = hints.begin(); i != hints.end(); i++)
	{
		string key;
		vector<string> values;

		if(MCOPUtils::tokenize(*i, key, values))
		{
			if(values.size() == 1)
				d->hints[key] = values[0];
		}
	}
}

string Connection::findHint(const string& hint)
{
	return d->hints[hint];
}
