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

#ifndef ASYNCSTREAM_H
#define ASYNCSTREAM_H

#include "buffer.h"
#include "datapacket.h"

/*
 * BC - Status (2000-09-30): GenericAsyncStream, AsyncStream,
 *   FloatAsyncStream/ByteAsyncStream
 *
 * These classes are to be treated with extreme care, as they are used in
 * all kinds of relations with the flow system and the generated code. Do
 * NOT touch unless you REALLY know what you are doing. For further
 * extensibility, GenericAsyncStream features a private d pointer.
 */

namespace Arts {

class GenericAsyncStreamPrivate;

class GenericAsyncStream {
private:
	GenericAsyncStreamPrivate *d;	// unused
public:
	/**
	 * interface to create packets and to get rid of them
	 */
	virtual GenericDataPacket *createPacket(int capacity) = 0;
	virtual void freePacket(GenericDataPacket *packet) = 0;

	virtual GenericAsyncStream *createNewStream() = 0;

	GenericDataChannel *channel;
	int _notifyID;

	inline int notifyID() { return _notifyID; }
};

template<class T>
class AsyncStream : public GenericAsyncStream {
protected:
	GenericDataPacket *createPacket(int capacity)
	{
		return allocPacket(capacity);
	}
	void freePacket(GenericDataPacket *packet)
	{
		delete packet;
	}
public:
	// for outgoing streams
	virtual DataPacket<T> *allocPacket(int capacity) = 0;

	inline void setPull(int packets, int capacity)
	{
		channel->setPull(packets,capacity);
	}
	inline void endPull()
	{
		channel->endPull();
	}
};

class FloatAsyncStream : public AsyncStream<float>
{
public:
	DataPacket<float> *allocPacket(int capacity);
	GenericAsyncStream *createNewStream();
};

class ByteAsyncStream : public AsyncStream<mcopbyte>
{
public:
	DataPacket<mcopbyte> *allocPacket(int capacity);
	GenericAsyncStream *createNewStream();
};

};

#endif /* ASYNCSTREAM_H */
